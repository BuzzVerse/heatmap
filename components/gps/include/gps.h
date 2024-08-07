#include <inttypes.h>

void gps_get_pos(int32_t *lat, int32_t *lon);
void gps_warm_start();
void gps_task(void *params);
int32_t gps_get_lat(void);
int32_t gps_get_lon(void);
uint16_t gps_get_alt(void);

