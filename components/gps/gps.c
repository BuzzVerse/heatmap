#include <stdio.h>
#include <string.h>
#include "gps.h"
#include <sys/unistd.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN CONFIG_GPS_EXAMPLE_PIN_TXD
#define RXD_PIN CONFIG_GPS_EXAMPLE_PIN_RXD
#define READ_COOLDOWN 10000
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



void get_gps_data(char** coordinates){
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, READ_COOLDOWN / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            //get only $GNGLL line
            *coordinates = strstr((char *)data, "$GNGLL");
            strtok(*coordinates, "\n");
            ESP_LOGI(TAG, "Read %d bytes:\n '%s'", rxBytes, (char *)data);
            ESP_LOGI(TAG, "GNGLL LINE: %s", *coordinates);

        //vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
    free(data);
}



