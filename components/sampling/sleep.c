#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp32/rom/uart.h"
#include "driver/rtc_io.h"

void GoToLightSleep(){
    esp_sleep_enable_timer_wakeup(20000000);
    
    // ToDo: Uncomment below in production code
    // esp_sleep_enable_timer_wakeup(SAMPLE_PERIOD_MS * 1000);

    // ToDo: Comment out below in production code
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_light_sleep_start();
}