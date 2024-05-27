#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "gps.h"
#include "lora.h"
#include <string.h>


//temp
#define PACKET_VERSION "1"
#define DEVICE_ID "HEATMAP"
#define MESSAGE_ID 1
#define MESSAGE_COUNT 1
#define DATA_TYPE "uint8_t"


static const char *MAIN_TAG = "Main";
//path to txt in sd card
const char *path = MOUNT_POINT"/gps.txt";
//for gps
static const int RX_BUF_SIZE = 1024;

void transmitter_task(){
    char* coordinates = (char*) malloc(RX_BUF_SIZE+1);
    lora_packet_t packet;
    packet.version = (uint8_t) PACKET_VERSION;
    packet.id = (uint8_t) DEVICE_ID;
    packet.msgID = (uint8_t) MESSAGE_ID;
    packet.msgCount = (uint8_t) MESSAGE_COUNT;
    packet.dataType = (uint8_t) DATA_TYPE;

    while (1) {
        get_gps_data(&coordinates);
        //ESP_LOGI(MAIN_TAG,"%s",coordinates);
        packet.data[0] = coordinates;
        lora_send(&packet);
    }
    free(coordinates);
}


void app_main(void){
    spi_init();
    lora_status_t ret = lora_init();
    if(ret==LORA_OK){
        //sdspi_init();
        uart_init();
        //gps_cold_start();

        transmitter_task();

        sdspi_close();
    }

}