#include "common.h"
#include "provisioning.h"

static char* ProvisionTAG = "ProvisionTask";

static bool wifi_provisioned()
{  
  if (ssid_provisioned /*&& user_provisioned */ && pass_provisioned){
    return true;
  }
  else{return false;}
}

static void ProvisionTask(void *para)
{
    initializeBLE();

    while(wifi_provisioned() == false)
    {
        ESP_LOGI(ProvisionTAG, "WiFi information not yet given...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    switch (prvsnState)
    {
        case InitialTry:
        wifiInit();
        //wifi_wpa2enterprise_initialize();
        break;

        case Retry:
        //ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
        ESP_ERROR_CHECK( esp_wifi_start() );
        break;
    }

    while(network_is_alive() == false)
    {
        ESP_LOGI(ProvisionTAG, "WiFi not yet connected...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while(write_key_provisioned == false)
    {
        ESP_LOGI(ProvisionTAG, "Write key not yet given...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    xSemaphoreGive(ProvisionTaskFlag);

    vTaskDelete(NULL);
}

void beginProvisioning()
{
    esp_log_level_set(ProvisionTAG, ESP_LOG_INFO);
    ProvisionTaskFlag = xSemaphoreCreateBinary();
    beginSamplingSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(&ProvisionTask, "Provision Task", 1024*5, NULL, 3, NULL);
}

bool ProvisionTaskDone()
{
  if (xSemaphoreTake(ProvisionTaskFlag, portMAX_DELAY) == pdTRUE)
  {
    ESP_LOGI(ProvisionTAG, "Provision Task finished!\n");
    return true;
  }
  else{return false;}
}