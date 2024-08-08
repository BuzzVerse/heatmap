#include <inttypes.h>

void gps_get_pos(int32_t *lat, int32_t *lon, uint16_t *alt);
void gps_get_status(uint8_t *status);
void gps_warm_start();
void gps_task(void *params);
