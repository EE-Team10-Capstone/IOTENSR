#include <stdio.h>
#include "sampling.h"
#include "i2c.h"
#include "scd41.h"
#include "thingspeak.h"
#include "sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define SamplingTaskTAG "SamplingTask"

static void SamplingTask(void *param)
{
    initializeSleep();

    initializeI2C();

    // ESP_ERROR_CHECK(stop_periodic_measurement());

    initializeThingSpeak();

    while(true)
    {   
        uint16_t co2;
        float temperature;
        float humidity;

        //ESP_ERROR_CHECK(measure_single_shot());

        ESP_ERROR_CHECK(start_periodic_measurement());

        bool data_ready = false;
        while(!data_ready)
        {
            ESP_ERROR_CHECK(get_data_ready_status(&data_ready));
            vTaskDelay(pdMS_TO_TICKS(SGNL_UPDT_INTRVL));
        }

        ESP_ERROR_CHECK(read_measurement(&co2, &temperature, &humidity));

        while (ThingSpeakPostData(&co2, &temperature, &humidity) != ESP_OK)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            ESP_LOGI(SamplingTaskTAG, "Waiting for data upload...\n");
        }

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