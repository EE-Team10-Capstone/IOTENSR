#include "common.h"
#include "provisioning.h"

static char* ProvisionTAG = "ProvisionTask";

static bool provisioned(){
  
  if (ssid_provisioned && /*user_provisioned &&*/ pass_provisioned /*&& write_key_provisioned*/){
    return true;
  }
  else{return false;}
  
}

static void ProvisionTask(void *para){

   ble_init(); 

   while(provisioned() == false)
   {
        ESP_LOGI(ProvisionTAG, "Information not yet given...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
   }


    wifiInit();
    //wifi_wpa2enterprise_initialize();


   while(network_is_alive() == false)
   {
        ESP_LOGI(ProvisionTAG, "WiFi not yet connected...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
   }

    xSemaphoreGive(ProvisionTaskFlag);

    vTaskDelete(NULL);
}

void beginProvisioning()
{
    esp_log_level_set(ProvisionTAG, ESP_LOG_INFO);
    ProvisionTaskFlag = xSemaphoreCreateBinary();
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