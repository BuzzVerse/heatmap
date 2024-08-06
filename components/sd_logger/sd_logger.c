#include "sd_logger.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_host.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdmmc_cmd.h"

#define TAG "SD_LOGGER"
#define SD_ALLOCATION_UNIT_SIZE 16 * 1024
#define SD_MAX_OPENED_FILES 5
#define SD_MOUNT_POINT "/sdcard"
#define SD_DATA_FILENAME "data.csv"
#define SD_DATA_FILE_HEADER "Timestamp,Fix,Lat,Lon,Alt,RSSI,CRC\n"

static sdmmc_card_t *card;
static spi_device_handle_t sd_spi;

static esp_err_t tool_sd_create_and_initialize_data_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return ESP_FAIL;
    }
    fprintf(file, SD_DATA_FILE_HEADER);
    fclose(file);

    return ESP_OK;
}

static esp_err_t tool_sd_static_append_record_to_file(
    const char *filename, const gps_record_t *record) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening file for appending");
        return ESP_FAIL;
    }

    int status = fprintf(file, "%d,%d,%d,%d,%d,%d,%d, %d\n", record->timestamp,
                         record->fix, record->lat, record->lon, record->alt,
                         record->spd, record->rssi, record->crc_error);

    fclose(file);

    if (status < 0) {
        ESP_LOGE(TAG, "Failed to write record to file");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static bool tool_sd_file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

static esp_err_t sd_mount() {
    ESP_LOGI(TAG, "Mounting filesystem");

    static sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = CONFIG_SPI_HOST;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CONFIG_SPI_UEXT_CS_PIN_NUM;
    slot_config.host_id = CONFIG_SPI_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_OPENED_FILES,
        .allocation_unit_size = SD_ALLOCATION_UNIT_SIZE};

    esp_err_t ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config,
                                            &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG,
                 "Failed to mount filesystem. If you want the card to be "
                 "formatted, set the format_if_mount_failed to true.");
        return ret;
    }

    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

static esp_err_t sd_init_spi_device() {
    spi_device_interface_config_t spi_dev_config_sd = {
        .clock_speed_hz = CONFIG_SPI_UEXT_SPEED,
        .mode = CONFIG_SPI_UEXT_MODE,
        .spics_io_num = CONFIG_SPI_UEXT_CS_PIN_NUM,
        .queue_size = 1,
    };

    // Add SD device to the list of SPI devices.
    esp_err_t ret =
        spi_bus_add_device(CONFIG_SPI_HOST, &spi_dev_config_sd, &sd_spi);
    if (ret != ESP_OK) {
        ESP_LOGE(
            TAG,
            "Failed to add SD SPI device. Is SD CS in used by someone else?");
        return ret;
    } else {
        ESP_LOGI(TAG, "SD SPI device added.");
    }

    return ESP_OK;
}

// Main thread for GPS logging
static void gps_logger_task(void *pvParameters) {
    QueueHandle_t gps_queue = (QueueHandle_t)pvParameters;
    gps_record_t gps_data;

    while (1) {
        if (xQueueReceive(gps_queue, &gps_data, portMAX_DELAY)) {
            // Handle received data - store to SD card.
            if (ESP_OK != tool_sd_static_append_record_to_file(
                              SD_MOUNT_POINT "/" SD_DATA_FILENAME, &gps_data)) {
                ESP_LOGE(TAG, "Failed to append record to file");
            }
            ESP_LOGI(TAG, "Record appended to file: %d,%d,%d,%d,%d,%d,%d,%d\n",
                     gps_data.timestamp, gps_data.fix, gps_data.lat,
                     gps_data.lon, gps_data.alt, gps_data.spd, gps_data.rssi,
                     gps_data.crc_error);
        }
    }
}

QueueHandle_t sd_logger_init() {
    if (ESP_OK != sd_init_spi_device()) {
        ESP_LOGE(TAG, "Failed to initialize SD SPI device");
        return NULL;
    }

    ESP_LOGD(TAG, "SD SPI device initialized");

    if (ESP_OK != sd_mount()) {
        ESP_LOGE(TAG, "Failed to mount SD card");
        return NULL;
    }

    ESP_LOGD(TAG, "SD card mounted");

    if (!tool_sd_file_exists(SD_MOUNT_POINT "/" SD_DATA_FILENAME)) {
        // If the file does not exist, create and initialize it
        tool_sd_create_and_initialize_data_file(SD_MOUNT_POINT
                                                "/" SD_DATA_FILENAME);
    }

    QueueHandle_t gps_queue = xQueueCreate(10, sizeof(gps_record_t));
    if (gps_queue == NULL) {
        printf("Failed to create queue\n");
        return NULL;
    }

    if (xTaskCreate(gps_logger_task, "gps_logger_task", 2048, (void *)gps_queue,
                    5, NULL) != pdPASS) {
        printf("Failed to create task\n");
        vQueueDelete(gps_queue);
        return NULL;
    }

    return gps_queue;
}
