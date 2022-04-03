#ifndef MAIN_SSD1366_H_
#define MAIN_SSD1366_H_

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#define SDA_PIN 18
#define SCL_PIN 19

void ssd1306_init();
void task_ssd1306_display_pattern(void *ignore);
void task_ssd1306_display_clear(void *ignore);
void task_ssd1306_contrast(void *ignore);
void task_ssd1306_scroll(void *ignore);
void task_ssd1306_display_text(void *arg_text);
void oled_main(void);
void display_char();

#endif /* MAIN_SSD1366_H_ */


