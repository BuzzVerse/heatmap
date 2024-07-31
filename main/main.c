#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "gps.h"
#include "lora.h"
#include <string.h>

#define TAG "Main"
#define PROJECT_VER "0.0.0"
#define PROJECT_NAME "BuzzVerse"

//for gps

#if 0
void transmitter_task(){
    char* coordinates = (char*) malloc(RX_BUF_SIZE+1);
    packet_t packet;
    packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
    packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
    packet.msgID = 1;                   // Example message ID
    packet.msgCount = 1;                // Example message count (optional, set as needed)
    packet.dataType = CONFIG_DATA_TYPE; // Example data type

    while (1) {
        get_gps_data(coordinates);
        ESP_LOGI(TAG,"%s",coordinates);
        packet.data[0] = (uint8_t) coordinates[0];
        lora_status_t send_status = lora_send(&packet);
        if (LORA_OK != send_status)
        {
            ESP_LOGE(TAG, "Packet send failed");
        }
        else
        {
            ESP_LOGI(TAG, "Packet sent successfully");
        }
        lora_delay(1000);
    }
    free(coordinates);
}

#endif
void app_main(void){
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;

#if 0
  lora_driver_init();
  lora_dump_registers();
    
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
			  lora_task,       /* Function that implements the task. */
			  "LoRa",          /* Text name for the task. */
			  1024,            /* Stack size in words, not bytes. */
			  ( void * ) 1,    /* Parameter passed into the task. */
			  tskIDLE_PRIORITY,/* Priority at which the task is created. */
			  &xHandle );      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS ) {
    vTaskDelete( xHandle );
    ESP_LOGE(TAG, "[FATAl] Could not create LoRa task!");
  }
#endif

#if 0
  lora_close();
#endif
}
