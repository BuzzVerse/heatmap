#include <inttypes.h>

void gps_get_pos(int32_t *lat, int32_t *lon);
void gps_cold_start();
void gps_task(void *params);
