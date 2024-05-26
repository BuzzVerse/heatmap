#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "gps.h"
#include <string.h>
static const char *TAG = "Main";
//path to txt in sd card
const char *path = MOUNT_POINT"/gps.txt";
//for gps
static const int RX_BUF_SIZE = 1024;

void transmitter_task(){
    char* coordinates = (char*) malloc(RX_BUF_SIZE+1);
    while (1) {
        get_gps_data(&coordinates);
        ESP_LOGI(TAG,"%s",coordinates);
    }
    free(coordinates);
}


void app_main(void){
    sdspi_init();
    uart_init();
    //gps_cold_start();

    transmitter_task();

    sdspi_close();
}