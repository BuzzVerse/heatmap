#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "gps.h"
#include <sys/unistd.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "GPS";
#define GPS_DBG_MODE 0

#define TXD_PIN CONFIG_GPS_EXAMPLE_PIN_TXD
#define RXD_PIN CONFIG_GPS_EXAMPLE_PIN_RXD
#define READ_COOLDOWN 10000


#define GPS_BUF_SZ 1024
#define NMEA_NUM_HANDLERS 2
#define GPS_NMEA_GNGGA "$GNGGA,"
#define GPS_NMEA_GNZDA "$GNZDA,"
#define GPS_NMEA_GNGGA_SZ 7
#define GPS_NMEA_GNZDA_SZ 7

typedef enum {
  GPS_OK = 0,
  GPS_ERROR,
  GPS_NMEA_OK,
  GPS_NMEA_FAIL
} gps_rc_t;

typedef struct {
  uint32_t time;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} gps_time_t;

typedef struct {
  int32_t lat;
  int32_t lon;
  char lat_hemisphere;
  char lon_hemisphere;
  uint16_t altitude;
} gps_pos_t;

typedef struct {
  gps_time_t utc;
  gps_pos_t pos;
  uint8_t num_sat;
  uint8_t quality;
  uint16_t hdop;
  bool fresh;
  uint8_t data[GPS_BUF_SZ];
} gps_t;

static gps_t gps = { 0 };

typedef struct {
  char *sentence;
  gps_rc_t (*handler)(char *);
} gps_nmea_handler_t;

static int32_t nmea_extract_value(char **sentence);
static char nmea_extract_hemispere(char **sentence);
static void print_gps_struct(void);

/* NMEA sentence handlers */
static gps_rc_t nmea_gngga_handler(char *sentence);
static gps_rc_t nmea_gnzda_handler(char *sentence);

static gps_nmea_handler_t gps_nmea_handlers[NMEA_NUM_HANDLERS] = {
  { GPS_NMEA_GNGGA, &nmea_gngga_handler },
  { GPS_NMEA_GNZDA, &nmea_gnzda_handler }
};

/*
 * Extracts value as integer from string e.g. "5156.42755," becomes value 515642755
 */
static int32_t nmea_extract_value(char **sentence)
{
  int32_t value = 0;

  if (NULL == sentence)
    return -1;

  for (value = 0; **sentence != ','; (*sentence)++) {
    switch (**sentence) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      value *= 10;
      value += (**sentence - '0');
      break;
      
    case '.':
      break;

    default:
      break;
    }
  }
  (*sentence)++;

  return value;
}

/*
 * $GNGLL,5156.42755,N,01531.78916,E,163017.000,A,A*44
 * Extracts N,S and E,W for specific hemisphere
 */

static char nmea_extract_hemispere(char **sentence)
{
  char hemisphere = 'H';

  if (NULL != *sentence) {
    switch (**sentence) {
    case 'N':
    case 'S':
    case 'E':
    case 'W':
      hemisphere = **sentence;
      *sentence += 2; /* Set pointer to value after comma */
      break;
    }
  }
  return hemisphere;
}

/*
 *
 */
static void print_gps_struct(void)
{
  ESP_LOGI(TAG, "**********************************************");
  ESP_LOGI(TAG, "* GPS data is %s", gps.fresh?"FRESH":"OLD");
  ESP_LOGI(TAG, "* [ UTC Time: %ld %d/%d/%d ]", gps.utc.time, gps.utc.day, gps.utc.month, gps.utc.year);
  if (0 == gps.quality) {
    ESP_LOGI(TAG, "* [ GPS quality: position fix unavailable ]");    
  } else {
    ESP_LOGI(TAG, "* [ GPS quality: %s ]", (gps.quality == 1)?"valid position fix, SPS mode":"valid position fix, differential GPS mode");
  }
  ESP_LOGI(TAG, "* [ Latitude: %ld %c ] [ Longitude: %ld %c ]",
	   gps.pos.lat, gps.pos.lat_hemisphere, gps.pos.lon, gps.pos.lon_hemisphere);
  ESP_LOGI(TAG, "* [ Altitude: %d ]", gps.pos.altitude);
  ESP_LOGI(TAG, "* [ HDOP: %d ]", gps.hdop);
  ESP_LOGI(TAG, "* [ Number of satelites: %d ]", gps.num_sat);
  ESP_LOGI(TAG, "**********************************************");

#if GPS_DBG_MODE == 1
  ESP_LOGI(TAG, "##############################################");
  ESP_LOGI(TAG, "# NMEA: %s", gps.data);
  ESP_LOGI(TAG, "##############################################");
#endif
}

/*
 * $GNGGA,180651.000,5156.44654,N,01531.81015,E,1,15,1.1,190.2,M,39.1,M,,*47
 */
static gps_rc_t nmea_gngga_handler(char *sentence)
{
  gps_rc_t rc = GPS_NMEA_FAIL;

  if (NULL != sentence) {
    sentence += GPS_NMEA_GNGGA_SZ;

    nmea_extract_value(&sentence); /* Drop time. Collected elsewhere. */
    gps.pos.lat = nmea_extract_value(&sentence);
    gps.pos.lat_hemisphere = nmea_extract_hemispere(&sentence);
    gps.pos.lon = nmea_extract_value(&sentence);
    gps.pos.lon_hemisphere = nmea_extract_hemispere(&sentence);
    gps.quality = nmea_extract_value(&sentence);
    gps.num_sat = nmea_extract_value(&sentence);
    gps.hdop = nmea_extract_value(&sentence);
    gps.pos.altitude = nmea_extract_value(&sentence);

    rc = GPS_NMEA_OK;
  }

  return rc;
}

/*
 * $GNZDA,140055.000,06,08,2024,00,00*47
 * Responsible for extracting UTC time, day, month, year.
 */
static gps_rc_t nmea_gnzda_handler(char *sentence)
{
  gps_rc_t rc = GPS_NMEA_FAIL;

  if (NULL != sentence) {
    sentence += GPS_NMEA_GNZDA_SZ;
    gps.utc.time = nmea_extract_value(&sentence);
    gps.utc.day = nmea_extract_value(&sentence);
    gps.utc.month = nmea_extract_value(&sentence);
    gps.utc.year = nmea_extract_value(&sentence);
    rc = GPS_NMEA_OK;
  }

  return rc;
}

/*
 * Main GPS task for handling GPS NMEA stream and extract its possition.
 */
void gps_task(void *params)
{
  char *ptr = NULL;
  uint8_t i;
  
  while (1) {

    if (0 < uart_read_bytes(UART_NUM_1, gps.data, GPS_BUF_SZ, READ_COOLDOWN / portTICK_PERIOD_MS)) {
      for (i = 0; i < NMEA_NUM_HANDLERS; i++) {
	ptr = strstr((char *)gps.data, gps_nmea_handlers[i].sentence);
	if (NULL != ptr) {
	  if (GPS_NMEA_OK == gps_nmea_handlers[i].handler(ptr)) {
	    gps.fresh = true;
	  }
	}
      }
    }
    vTaskDelay(100);
    if (true == gps.fresh) {
#if 0
      if (pdTrue == xQueueSend(msg, sdcard_Q, etc... )) {
	gps.fresh = false;
      }
#else
      gps.fresh = false;
      
#endif 
    }
    print_gps_struct();
  }
}

static void uart_init(void)
{
    const uart_config_t uart_config ={
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    if (ESP_OK == uart_driver_install(UART_NUM_1, GPS_BUF_SZ, 0, 0, NULL, 0)) {
      ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
      ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    } else {
      ESP_LOGE(TAG, "GPS UART driver install failed");
    }
}

/*
 * This is warm start not cold!
 */
void gps_cold_start(void){
  char* data = "$PCAS10,1*1A"; // TBD Here we have to be smart. When we do cold or warm start?!

  uart_init();
  vTaskDelay(500);
  ESP_LOGI(TAG, "GPS cold start %s", data);
  uart_write_bytes(UART_NUM_1, data, 100);
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

