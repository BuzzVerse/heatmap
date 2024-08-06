#ifndef __SD_LOGGER_H__
#define __SD_LOGGER_H__

#include "driver/spi_master.h"
#include "freertos/queue.h"

typedef struct {
    int timestamp;
    int fix;
    int lat;
    int lon;
    int alt;
    int spd;
    int rssi;
    int crc_error;
} gps_record_t;

QueueHandle_t sd_logger_init();

#endif  // __SD_LOGGER_H__
