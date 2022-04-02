#include "sleep.h"
#include "common.h"
#include "provisioning.h"

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

#define SleepTAG "Light Sleep"
#define SLEEP_PERIOD_MS 60*1000*1000 // 1 min sample period
#define PUSHBUTTON 2

static esp_sleep_wakeup_cause_t WakeUpCause;

void initializeSleep()
{
    ESP_LOGI(SleepTAG, "Initializing sleep mode params\n");

    if(esp_sleep_is_valid_wakeup_gpio(PUSHBUTTON)){printf("Valid GPIO\n");}
    else
    {
        ESP_LOGE(SleepTAG, "Non-vaild GPIO\n");
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
    esp_sleep_enable_timer_wakeup(SLEEP_PERIOD_MS);
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

bool PushButtonLongPress()
{   
    uint8_t i = 0;
    while(rtc_gpio_get_level(PUSHBUTTON) == 0)
    {
        if (i++ == 3) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    return false;
}


void GoToLightSleep()
{   
    esp_wifi_stop();
    
    sleep_time = esp_timer_get_time();

    // ToDo: Comment out below in production code
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_light_sleep_start();

    wake_time = esp_timer_get_time();

    ESP_LOGI(SleepTAG, "Slept for: %llds\n", (wake_time - sleep_time) / 1000000);
}



void WakeUpRoutine()
{
    WakeUpCause = esp_sleep_get_wakeup_cause();

    if(WakeUpCause == ESP_SLEEP_WAKEUP_GPIO)
    {
        ESP_LOGI(SleepTAG, "Pushbutton wakeup!\n");
        
        uint8_t i = 0;
        while(i++ < 5)
        {
            if (rtc_gpio_get_level(PUSHBUTTON) == 0){break;}
            ESP_LOGI(SleepTAG, "Waiting for long press.\n");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        if(PushButtonLongPress())
        {
            prvsnState = Retry;
            beginProvisioning();

            while(!ProvisionTaskDone())
            {
                printf("Provision Task in progress.\n");
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }

        wake_time = esp_timer_get_time();
        ESP_LOGI(SleepTAG, "Adjusted sleeptime: %llds\n", (SLEEP_PERIOD_MS - (wake_time - sleep_time)) / 1000000);
        esp_sleep_enable_timer_wakeup(SLEEP_PERIOD_MS - (wake_time - sleep_time));

        GoToLightSleep();

        esp_sleep_enable_timer_wakeup(SLEEP_PERIOD_MS);
    }

    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
    ESP_ERROR_CHECK( esp_wifi_start() );

    while(network_is_alive() == false)
    {
        ESP_LOGI("WiFi", "WiFi not yet connected...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}
