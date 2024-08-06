// lora.c

#include "lora.h"
#include "driver/lora_driver_defs.h"
#include "driver/lora_driver.h"
#include "protocols/packet/packet.h"
#include "api/driver_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <string.h>
#include <stdio.h>

// Remove if wanting to make lora.c HAL independent
#include "esp_system.h"

#define LORA_FREQUENCY (CONFIG_LORA_FREQUENCY * 1e6)
#define LORA_CODING_RATE CONFIG_LORA_CODING_RATE
#define LORA_BANDWIDTH CONFIG_LORA_BANDWIDTH
#define LORA_SPREADING_FACTOR CONFIG_LORA_SPREADING_FACTOR
#define LORA_CRC CONFIG_LORA_CRC

#define SEND_TIMEOUT 15000 // Adjusted to milliseconds
#define CONFIRMATION_TIMEOUT 5000 // Adjusted to milliseconds

TimerHandle_t sendTimer;
volatile bool timeout_occurred = false;

TaskHandle_t sendTaskHandle = NULL;
TaskHandle_t mainTaskHandle = NULL;

void pack_packet(uint8_t *buffer, packet_t *packet);
void print_buffer(uint8_t *buffer, size_t size);

lora_status_t lora_send(packet_t *packet)
{
    uint8_t buffer[PACKET_SIZE] = {0};

    pack_packet(buffer, packet);
    print_buffer(buffer, sizeof(buffer));

    // Send the packet using the HAL function
    lora_send_packet(buffer, sizeof(buffer));

    return LORA_OK;
}

void lora_get_config(void)
{
  uint8_t cr = 0, sbw = 0, sf = 0, val = 0;
  
  if (LORA_OK == lora_get_coding_rate(&cr)) {
    printf("CR: 0x%x\n", cr);
  } else {
    printf("CR: ERROR\n");
  }

  if (LORA_OK == lora_get_bandwidth(&sbw)) {
    printf("SBW: 0x%x\n", sbw);
  } else {
    printf("SBW: ERROR\n");
  }

  if (LORA_OK == lora_get_spreading_factor(&sf)) {
    printf("SF: 0x%x\n", sf);
  } else {
    printf("SF: ERROR\n");
  }

  if (ESP_OK == spi_read(REG_FRF_MSB, &val)) {
    printf("FREQ MSB: 0x%x\n", val);
  } else {
    printf("FREQ MSB: ERROR\n");
  }

  if (ESP_OK == spi_read(REG_FRF_MID, &val)) {
    printf("FREQ MSB: 0x%x\n", val);
  } else {
    printf("FREQ MSB: ERROR\n");
  }

  if (ESP_OK == spi_read(REG_FRF_LSB, &val)) {
    printf("FREQ MSB: 0x%x\n", val);
  } else {
    printf("FREQ MSB: ERROR\n");
  }
}

void pack_packet(uint8_t *buffer, packet_t *packet)
{
    buffer[0] = packet->version;
    buffer[1] = packet->id;
    buffer[2] = packet->msgID;
    buffer[3] = packet->msgCount;
    buffer[4] = packet->dataType;
    memcpy(&buffer[META_DATA_SIZE], packet->data, DATA_SIZE);
}

void print_buffer(uint8_t *buffer, size_t size)
{
    printf("\n");
    for (size_t i = 0; i < size; i++)
    {
        printf("0x%x ", buffer[i]);
    }
    printf("\n");
}

void sendPacketTimeoutHandler(TimerHandle_t xTimer)
{
    printf("Send packet timeout.\n");
    timeout_occurred = true;

    if (sendTaskHandle != NULL)
    {
        vTaskDelete(sendTaskHandle);
    }

    lora_sleep_mode();
    lora_write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);

    xTaskNotifyGive(mainTaskHandle);
}

void noConfirmationHandler(TimerHandle_t xTimer)
{
    printf("Confirmation timeout.\n");
    timeout_occurred = true;
    xTimerStop(xTimer, 0);
}

lora_status_t lora_init(void)
{
    printf("FREQ: %f\n", LORA_FREQUENCY);
    printf("CODING RATE: %d\n", LORA_CODING_RATE);
    printf("BANDWIDTH: %d\n", LORA_BANDWIDTH);
    printf("SPREADING FACTOR: %d\n", LORA_SPREADING_FACTOR);
    printf("CRC: %d\n", LORA_CRC);

    if (LORA_OK != lora_driver_init())
    {
        printf("LoRa initialization failed\n");
        esp_restart();
    }

    lora_set_frequency(LORA_FREQUENCY);
    lora_set_bandwidth(LORA_BANDWIDTH);
    lora_set_coding_rate(LORA_CODING_RATE);
    lora_set_spreading_factor(LORA_SPREADING_FACTOR);
    if (LORA_CRC)
    {
        lora_enable_crc();
    }

    lora_idle_mode(); // moved from lora_driver_init()

    return LORA_OK;
}
#if 0
lora_status_t lora_send(packet_t *packet)
{

    sendTimer = xTimerCreate("SendTimer", pdMS_TO_TICKS(SEND_TIMEOUT), pdFALSE, (void *)0, sendPacketTimeoutHandler);

    if (sendTimer == NULL)
    {
        printf("Failed to create timer.\n");
        return LORA_FAILED_SEND_PACKET;
    }

    mainTaskHandle = xTaskGetCurrentTaskHandle();

    BaseType_t xReturned = xTaskCreate(lora_send_task, "LoRaSendTask", 2048, (void *)packet, tskIDLE_PRIORITY, &sendTaskHandle);

    if (xReturned != pdPASS)
    {
        printf("Failed to create send task.\n");
        return LORA_FAILED_SEND_PACKET;
    }

    if (xTimerStart(sendTimer, 0) != pdPASS)
    {
        printf("Failed to start timer.\n");
        return LORA_FAILED_SEND_PACKET;
    }

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    xTimerStop(sendTimer, 0);

    if (timeout_occurred)
    {
        return LORA_FAILED_SEND_PACKET;
    }

    return LORA_OK;

}
#endif
lora_status_t lora_receive(packet_t *packet)
{
    uint8_t buffer[PACKET_SIZE] = {0};
    uint8_t length = 0;

    lora_receive_mode();

    while (1)
    {
        bool hasReceived = false;
        bool crc_error = false;
        lora_received(&hasReceived, &crc_error);

        if (hasReceived)
        {
            lora_receive_packet(buffer, &length, sizeof(buffer));

            print_buffer(buffer, sizeof(buffer));

            packet->version = buffer[0];
            packet->id = buffer[1];
            packet->msgID = buffer[2];
            packet->msgCount = buffer[3];
            packet->dataType = buffer[4];

            memcpy(packet->data, &buffer[META_DATA_SIZE], DATA_SIZE);
            return crc_error ? LORA_CRC_ERROR : LORA_OK;
        }
        lora_delay(20);
    }
}
