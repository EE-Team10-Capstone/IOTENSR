#include <stdio.h>


#include "sampling.h"
#include "i2c.h"
#include "scd41.h"
#include "thingspeak.h"
#include "sleep.h"
#include "common.h"

#define SamplingTaskTAG "SamplingTask"
#define ONE_WEEK 10 //2016 // 2016 * 5 minutes in one week
static uint16_t sample_counter;

static void SamplingTask(void *param)
{
    sample_counter = 0;
    initializeSleep();

    // initializeI2C();

    // ESP_ERROR_CHECK(stop_periodic_measurement());

    // initializeThingSpeak();

    while(xSemaphoreTake(beginSamplingSemaphore, portMAX_DELAY) == pdFALSE)
    {
        ESP_LOGI(SamplingTaskTAG, "Waiting for Begin Sampling flag...\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    deinitializeBLE();

    while(true)
    {   
        if (sample_counter++ == ONE_WEEK)
        {
            ESP_LOGI(SamplingTaskTAG, "One week reached; finishing task\n");
            GoToDeepSleep();
        }
        ESP_LOGI(SamplingTaskTAG, "Sample Counter: %d\n", sample_counter);

        // uint16_t co2;
        // float temperature;
        // float humidity;

        //ESP_ERROR_CHECK(measure_single_shot());

        // ESP_ERROR_CHECK(start_periodic_measurement());

        // bool data_ready = false;
        // while(!data_ready)
        // {
        //     ESP_ERROR_CHECK(get_data_ready_status(&data_ready));
        //     vTaskDelay(pdMS_TO_TICKS(SGNL_UPDT_INTRVL));
        // }

        // ESP_ERROR_CHECK(read_measurement(&co2, &temperature, &humidity));

        // while (ThingSpeakPostData(&co2, &temperature, &humidity) != ESP_OK)
        // {
        //     vTaskDelay(pdMS_TO_TICKS(1000));
        //     ESP_LOGI(SamplingTaskTAG, "Waiting for data upload...\n");
        // }

        ESP_LOGI(SamplingTaskTAG, "Entering Light Sleep!\n");
        GoToLightSleep();

        pushbuttonDebounce();

        WakeUpRoutine();
    }
}
    
void beginSampling(void)
{   
    esp_log_level_set(SamplingTaskTAG, ESP_LOG_INFO);
    ESP_LOGI(SamplingTaskTAG, "Beginning\n");
    xTaskCreate(&SamplingTask, "Provision Task", 1024*5, NULL, 3, NULL);
}