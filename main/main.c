#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_logger.h"
#include "gps.h"
#include "lora.h"
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"

#define TAG "Main"
#define PROJECT_VER "0.0.0"
#define PROJECT_NAME "BuzzVerse"


typedef union {
  int32_t deserialized;
  uint8_t serialized[4];
} coordinates_t;

void lora_task(){
    coordinates_t coordinates;
    packet_t packet;
    int32_t lat = 0, lon = 0;
    packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
    packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
    packet.msgID = 1;                   // Example message ID
    packet.msgCount = 1;                // Example message count (optional, set as needed)
    packet.dataType = CONFIG_DATA_TYPE; // Example data type

    vTaskDelay(100);
    lora_init();
    lora_dump_registers();
    vTaskDelay(100);

    while (1) {

      packet.data[0] = 8;

      coordinates.deserialized = lat;
      
      packet.data[1] = coordinates.serialized[0];
      packet.data[2] = coordinates.serialized[1];
      packet.data[3] = coordinates.serialized[2];
      packet.data[4] = coordinates.serialized[3];

      coordinates.deserialized = lon;

      packet.data[5] = coordinates.serialized[0];
      packet.data[6] = coordinates.serialized[1];
      packet.data[7] = coordinates.serialized[2];
      packet.data[8] = coordinates.serialized[3];
      
      lora_status_t send_status = lora_send(&packet);
      if (LORA_OK != send_status) {
	ESP_LOGE(TAG, "Packet send failed");
      } else {
	ESP_LOGI(TAG, "Packet sent successfully");
      }
      vTaskDelay(100);
      lora_get_config();
      vTaskDelay(500);
    }
}

static void init_spi_bus()
{
    // Setup SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_SPI_MOSI_PIN_NUM,
        .miso_io_num = CONFIG_SPI_MISO_PIN_NUM,
        .sclk_io_num = CONFIG_SPI_CLK_PIN_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize(CONFIG_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus");
        return;
    }
}

void app_main(void){
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;

  // Mandatory initialization of SPI bus, common for LoRa and UEXT module.
  init_spi_bus();

  // --- Example of sending data to SD logger ---
  
  // Get the SD logger queue
  QueueHandle_t sd_logger_queue = sd_logger_init();
  if (NULL == sd_logger_queue) {
    ESP_LOGE(TAG, "Failed to initialize SD logger");
    return;
  }

  gps_record_t sample_gps_data[] = {
    {1722963321, 0, -269434373, -1666061503, 5400, 111, -116, 0},
    {1722963322, 0, -579515899, 898377274, 8239, 92, -33, 0},
    {1722963323, 0, 612009894, 439500542, 4695, 191, -37, 0},
    {1722963324, 1, 69669419, 467416384, 4167, 162, -11, 0},
    {1722963325, 1, 135862440, -291246623, 5899, 112, -50, 1},
    {1722963326, 0, 40499433, -929341812, 5450, 243, -1, 1},
    {1722963327, 0, -291827279, 827895210, 6817, 171, -11, 0},
    {1722963328, 0, 239566285, 1351824774, 4178, 140, -60, 0},
    {1722963329, 1, 321526551, 20169793, 6455, 216, -99, 0},
    {1722963330, 0, -266104899, 429023739, 3600, 248, -120, 1},
  };

  int num_elements = sizeof(sample_gps_data) / sizeof(sample_gps_data[0]);
  
  for (int i = 0; i < num_elements; i++) {
    if (pdTRUE != xQueueSend(sd_logger_queue, &sample_gps_data[i], 0)) {
      ESP_LOGE(TAG, "Failed to send data to SD logger");
    }
    vTaskDelay(30);
  }

  // --- END of example ---

  gps_cold_start();
  xReturned = xTaskCreate(
			  &gps_task,       /* Function that implements the task. */
			  "GPS",           /* Text name for the task. */
			  4*1024,            /* Stack size in words, not bytes. */
			  ( void * ) 1,    /* Parameter passed into the task. */
			  tskIDLE_PRIORITY,/* Priority at which the task is created. */
			  &xHandle );      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS ) {
    vTaskDelete( xHandle );
    ESP_LOGE(TAG, "[FATAl] Could not create GPS task!");
  }

#if 1
    xReturned = xTaskCreate(
  			  &lora_task,      /* Function that implements the task. */
  			  "LoRa",          /* Text name for the task. */
  			  4*1024,          /* Stack size in words, not bytes. */
			  ( void * ) 1,    /* Parameter passed into the task. */
			  tskIDLE_PRIORITY,/* Priority at which the task is created. */
			  &xHandle );      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS ) {
    vTaskDelete( xHandle );
    ESP_LOGE(TAG, "[FATAl] Could not create LoRa task!");
  }
#endif

  printf("     ,     ,\n");
  printf("    (\\____/)\n");
  printf("     (_oo_)\n");
  printf("       (O)\n");
  printf("     __||__    \\)\n");
  printf("  []/______\\[] /\n");
  printf("  / \\______/ \\/\n");
  printf(" /    /__\\\n");
  printf("(\\   /____\\\n");

  while(1) {
    vTaskDelay(1000);
  }

#if 1
  lora_close();
#endif
}
