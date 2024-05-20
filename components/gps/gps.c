#include <stdio.h>
#include <string.h>
#include "gps.h"
#include <sys/unistd.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const int RX_BUF_SIZE = 2048;

#define TXD_PIN CONFIG_GPS_EXAMPLE_PIN_TXD
#define RXD_PIN CONFIG_GPS_EXAMPLE_PIN_RXD

static const char *TAG = "GPS";




void uart_init()
{
    const uart_config_t uart_config ={
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM_1, RX_BUF_SIZE*2, 0, 0, NULL, 0);
    ESP_ERROR_CHECK(
        uart_param_config(UART_NUM_1, &uart_config)
    );
    ESP_ERROR_CHECK(
        uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)
    );
}
void gps_cold_start(){
    char* data = "$PCAS10,3*1C";
    ESP_LOGI(TAG, "GPS cold start %s", data);
    uart_write_bytes(UART_NUM_1, data, RX_BUF_SIZE);
    vTaskDelay(30000 / portTICK_PERIOD_MS);
}



void get_gps_data(){
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 500 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(TAG, "Read %d bytes:\n '%s'", rxBytes, (char *)data);

        }
        //vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
    free(data);
}





//conversion -- TODO: catch empty data 
// UNUSED FOR NOW
// float convertToDegrees(char *raw, char *direction){
//     int sign=1;
//     if(strspn(direction, "N")==1
//     || strspn(direction, "S")==1){
//         if(strspn(direction, "N")!=1) sign = -1;
//         //latitude ddmm.mmmm
//         char degrees[3] = {raw[0], raw[1], '\0'};
//         char *minutes = &raw[2];
//         //printf("%s\n", minutes);
//         return (atof(degrees) + atof(minutes)/60)*sign;
//     }
//     if(strspn(direction, "W")==1
//     || strspn(direction, "E")==1){
//         if(strspn(direction, "W")==1) sign = -1;
//         //longitude dddmm.mmmm
//         char degrees[4] = {raw[0], raw[1],raw[2], '\0'};

//         char *minutes = &raw[3];
//         printf("%s\n", minutes);
//         return (atof(degrees) + atof(minutes)/60)*sign;
//     }
//     printf("Invalid direction"); //logw

//     return 0;
// }

// void decodeCoordinates(char* data){
//     //get $GPGLL line
//     char *coordinates = malloc(RX_BUF_SIZE);
//     coordinates = strstr(data, "$GPGLL");
//     unsigned int lineSize;
//     //lineSize = (unsigned int) strcspn(coordinates, "\n");
//     //get values
//     char *substr = malloc(RX_BUF_SIZE);
//     memcpy(substr, coordinates, RX_BUF_SIZE);
//     //char *messageId =
//             strtok(substr, ",");
//     char *rawLatitude = strtok(NULL, ",");
//     char *latDirection = strtok(NULL, ",");
//     char *rawLongitude = strtok(NULL, ",");
//     char *lonDirection = strtok(NULL, ",");
//     //char *utc =
//         strtok(NULL, ",");
//     char *dataStatus = strtok(NULL, ",");
//     //check data status and convert
//     printf("%s", dataStatus);
//     if(dataStatus[0]!='A'){
//         //ESP_LOGW
//         printf("Invalid data status");
//         return;
//     }
//     printf("%s", rawLatitude);

//     float lat = convertToDegrees(rawLatitude,  latDirection);
//     float lon = convertToDegrees(rawLongitude,  lonDirection);

//     //put coordinates into data string
//     char buffer[50];
//     sprintf(buffer,"%f %f", lat, lon);
//     strcpy(data, buffer);
//     //free memory
//     free(substr);
// }
