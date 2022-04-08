#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#define WAKE_UP_MSG "IOTENSR\n\nIoT\n\nEnvironment\n\nSensor"

void initializeOLED();
void printOLED(char *);
void printSample(uint16_t *co2, float *temperature, float *humidity);
