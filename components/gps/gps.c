#include <stdio.h>
#include <string.h>
#include "gps.h"
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const int RX_BUF_SIZE = 2024;

#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)

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

void get_gps_data(){
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 500 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(TAG, "Read %d bytes: '%s'", rxBytes, (char *)data);
        }
    }
    free(data);
}


