#define MOUNT_POINT "/sdcard"
#include "esp_check.h"

void sdspi_test(void);
void sdspi_init(void);
void sdspi_close(void);
void sdspi_write_file(const char *path, char *data);
