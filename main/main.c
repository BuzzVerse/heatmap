#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"


// void save_to_sdcard(){
//     sdspi_test();
//     while(1){
//         vTaskDelay(30000 / portTICK_PERIOD_MS);
//         sdspi_test();
//     }

// }

void app_main(void){

    sdspi_init();

    sdspi_test();  

    sdspi_close();

    //xTaskCreate(&save_to_sdcard, "sdspi_test_task", 2048, NULL, 5, NULL);

}