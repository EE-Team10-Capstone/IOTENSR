#include "sleep.h"

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

#define PUSHBUTTON 2

void initializeSleep()
{
    printf("Initializing Light Sleep mode parameters\n");

    if(esp_sleep_is_valid_wakeup_gpio(PUSHBUTTON)){printf("Valid GPIO\n");}
    else
    {
        printf("Non-valid GPIO\n");
        while(1)
        { 
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    gpio_pad_select_gpio(PUSHBUTTON);
    gpio_set_direction(PUSHBUTTON, GPIO_MODE_INPUT);
    gpio_pullup_en(PUSHBUTTON); gpio_pulldown_dis(PUSHBUTTON);

    gpio_wakeup_enable(PUSHBUTTON, GPIO_INTR_LOW_LEVEL);

    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(SLEEP_PERIOD);
}

void pushbuttonDebounce()
{
    if (rtc_gpio_get_level(PUSHBUTTON) == 0)
    {
        printf("Button debounce\n");
        do
        {
        vTaskDelay(pdMS_TO_TICKS(10));
        } while (rtc_gpio_get_level(PUSHBUTTON) == 0);
    }
}

void GoToLightSleep()
{
    sleep_time = esp_timer_get_time();

    // ToDo: Comment out below in production code
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_light_sleep_start();

    wake_time = esp_timer_get_time();

    printf("Slept for: %llds\n", (waketime - sleeptime) / 1000000);
}

void WakeUpLogic()
{
    esp_sleep_wakeup_cause_t WakeUpCause;

    WakeUpCause = esp_sleep_get_wakeup_cause();

    if(WakeUpCause == ESP_SLEEP_WAKEUP_GPIO)
    {
        printf("Pushbutton wake-up!\n");
        printf("Adjusted sleeptime: %llds\n", (SLEEP_PERIOD - (waketime - sleeptime)) / 1000000);
        esp_sleep_enable_timer_wakeup(SLEEP_PERIOD - (waketime - sleeptime));
    }
}