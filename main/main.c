#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "gps.h"
#include "lora.h"
#include <string.h>

#define TAG "Main"


//for gps
static const int RX_BUF_SIZE = 1024;

void transmitter_task(){
    char* coordinates = (char*) malloc(RX_BUF_SIZE+1);
    packet_t packet;
    packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
    packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
    packet.msgID = 1;                   // Example message ID
    packet.msgCount = 1;                // Example message count (optional, set as needed)
    packet.dataType = CONFIG_DATA_TYPE; // Example data type

    while (1) {
        get_gps_data(&coordinates);
        ESP_LOGI(TAG,"%s",coordinates);
        packet.data[0] = (uint8_t) coordinates;
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


void app_main(void){
    lora_driver_init();
    lora_dump_registers();
    // sdspi_init();
    // sdspi_test();
    uart_init();
    gps_cold_start();
    transmitter_task();
    lora_close();
}