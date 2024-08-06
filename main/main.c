#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "gps.h"
#include "lora.h"
#include <string.h>
#include <inttypes.h>

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
      gps_get_pos(&lat, &lon);

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


void app_main(void){
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;

#if 0
  
  //sdspi_init();
  //sdspi_test();
#endif
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

  printf("     ,     ,\n");
  printf("    (\\____/)\n");
  printf("     (_oo_)\n");
  printf("       (O)\n");
  printf("     __||__    \\)\n");
  printf("  []/______\\[] /\n");
  printf("  / \\______/ \\/\n");
  printf(" /    /__\\\n");
  printf("(\\   /____\\\n");

#if 0
  lora_close();
#endif
}
