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

#define TXD_PIN CONFIG_GPS_EXAMPLE_PIN_TXD
#define RXD_PIN CONFIG_GPS_EXAMPLE_PIN_RXD
#define READ_COOLDOWN 10000


#define GPS_BUF_SZ 1024
#define GPS_NMEA_GNGLL "$GNGLL,"
#define GPS_NMEA_GNGLL_SZ 7

typedef struct {
  int32_t lon;
  int32_t lat;
  float time;
  float speed;
  uint8_t num_sat;
  bool fix;
  uint8_t data[GPS_BUF_SZ];
} gps_t;

static gps_t gps = { 0 };

void gps_get_pos(int32_t *lon, int32_t *lat)
{
  if (NULL == lon || NULL == lat)
    return;

  *lon = gps.lon;
  *lat = gps.lat;
}

float gps_get_speed(void)
{
  return -1.0;
}

/*
 * Main GPS task for handling GPS NMEA stream and extract its possition.
 */
void gps_task(void *params)
{
  char *ptr = NULL;
  float lat, lon;
  
  while (1) {

    if (0 < uart_read_bytes(UART_NUM_1, gps.data, GPS_BUF_SZ, READ_COOLDOWN / portTICK_PERIOD_MS)) {
      ptr = strstr((char *)gps.data, GPS_NMEA_GNGLL);
      if (ptr) {
	ptr += GPS_NMEA_GNGLL_SZ;

	lat = atof(ptr);

	while (*ptr++ != ','); // TBD: Seek till comma. Need to failsafe here!
	while (*ptr++ != ','); // TBD: Seek till comma. Need to failsafe here!
	
	lon = atof(ptr);

	gps.lat = (int32_t)(lat * 10000.0);
	gps.lon = (int32_t)(lon * 10000.0);
	ESP_LOGI(TAG, "GNGLL position: [ latitude: %ld ], [ longitude: %ld ]", gps.lat, gps.lon);
	ESP_LOGI(TAG, "NMEA: %s", gps.data);
      }
    }
    vTaskDelay(100);
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

void gps_cold_start(void){
  char* data = "$PCAS10,1*1A"; // TBD Here we have to be smart. When we do cold or warm start?!

  uart_init();
  vTaskDelay(500);
  ESP_LOGI(TAG, "GPS cold start %s", data);
  uart_write_bytes(UART_NUM_1, data, 100);
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

