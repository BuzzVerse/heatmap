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

static void show_logo(void);

static void show_logo(void)
{
  ESP_LOGI(TAG, "     ,     ,");
  ESP_LOGI(TAG, "    (\\____/)");
  ESP_LOGI(TAG, "     (_oo_)");
  ESP_LOGI(TAG, "       (O)");
  ESP_LOGI(TAG, "     __||__    \\)");
  ESP_LOGI(TAG, "  []/______\\[] /");
  ESP_LOGI(TAG, "  / \\______/ \\/");
  ESP_LOGI(TAG, " /    /__\\");
  ESP_LOGI(TAG, "(\\   /____\\");
}

void lora_task(){
    coordinates_t coordinates;
    packet_t packet;
    packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
    packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
    packet.msgID = 1;                   // Example message ID
    packet.msgCount = 1;                // Example message count (optional, set as needed)
    packet.dataType = 4;

    vTaskDelay(100);
    //    lora_init();
    lora_dump_registers();
    vTaskDelay(100);

    while (1) {

      packet.data[0] = 11;
      packet.data[1] = 0;

      uint16_t altitude = gps_get_alt();
      
      packet.data[2] = altitude >> 8;
      packet.data[3] = (uint8_t)(0x00FF & altitude);

      
      coordinates.deserialized = gps_get_lat();
      
      packet.data[4] = coordinates.serialized[0];
      packet.data[5] = coordinates.serialized[1];
      packet.data[6] = coordinates.serialized[2];
      packet.data[7] = coordinates.serialized[3];

      coordinates.deserialized = gps_get_lon();

      packet.data[8] = coordinates.serialized[0];
      packet.data[9] = coordinates.serialized[1];
      packet.data[10] = coordinates.serialized[2];
      packet.data[11] = coordinates.serialized[3];
      
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
  QueueHandle_t sd_logger_queue;

  show_logo();

  // Mandatory initialization of SPI bus, common for LoRa and UEXT module.
  init_spi_bus();

  sd_logger_queue = sd_logger_init();
  if (NULL == sd_logger_queue) {
    ESP_LOGE(TAG, "Failed to initialize SD logger");
    return;
  }

  gps_warm_start();
  xReturned = xTaskCreate(
			  &gps_task,       /* Function that implements the task. */
			  "GPS",           /* Text name for the task. */
			  4*1024,          /* Stack size in words, not bytes. */
			  (void *)sd_logger_queue,/* Parameter passed into the task. */
			  tskIDLE_PRIORITY,/* Priority at which the task is created. */
			  &xHandle );      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS ) {
    vTaskDelete( xHandle );
    ESP_LOGE(TAG, "[FATAl] Could not create GPS task!");
  }

#if 0
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

#if 1

  while(1) {
    vTaskDelay(1000);
  }
#endif
#if 0
  lora_close();
#endif
}
