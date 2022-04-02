#include "common.h"
#include "esp_netif.h"

#define EAP_ID "anonymous@ualberta.ca"
#define PASS "Modestkarl"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

/* esp netif object representing the WIFI station */
static esp_netif_t *sta_netif = NULL;
static char *WIFI_TAG = "CONNECTION";

bool network_is_alive(void)
{
    EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
    if (uxBits & CONNECTED_BIT) {
      return true;
    } else {
      return false;
    }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        
      esp_wifi_connect();
      
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

      esp_wifi_connect();
      xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

      xSemaphoreGive(connectionSemaphore);
      xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

static void OnConnected(void *para)
{
  while (1)
  {
    if (xSemaphoreTake(connectionSemaphore, pdMS_TO_TICKS(15000)))
    {
      ESP_LOGI(WIFI_TAG, "Connected!\n");
      wififlag = 1;
      vTaskDelete(NULL);
      //xSemaphoreTake(connectionSemaphore, portMAX_DELAY);
    }
    else
    {
      ESP_LOGE(WIFI_TAG, "Failed to connect.\n");
      
      wififlag = 0;
      //user_provisioned = false;
      pass_provisioned = false;
      ssid_provisioned = false;
      printf("WiFi and provision flags cleared\n");

      while(!(user_provisioned && pass_provisioned && ssid_provisioned))
      {
        printf("Waiting for retry\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
      }

      ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
      vTaskDelay(pdMS_TO_TICKS(2000));
      ESP_ERROR_CHECK( esp_wifi_start() );
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void wifiInit()
{   
    printf("WiFi initializing!\n");
    ESP_ERROR_CHECK(nvs_flash_init());
    connectionSemaphore = xSemaphoreCreateBinary();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *netif_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,event_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,event_handler,NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    char homessid[] = "Ginger Beef";
    char homepassword[] = "hansandfranciswashere100%";
    strcpy((char *)wifi_config.sta.ssid, homessid);
    strcpy((char *)wifi_config.sta.password, homepassword);

    printf("WiFi configuring with:\n");
    printf("SSID: %s\n", wifi_config.sta.ssid);
    printf("Password: %s\n", wifi_config.sta.password);
 
    esp_wifi_set_mode(WIFI_MODE_STA);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(&OnConnected, "Connection Handler", 1024*3, NULL, 5, NULL);
    
    printf("WiFi initialized!\n");
}

void wifi_wpa2enterprise_initialize()
{   
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_event_group = xEventGroupCreate();
    connectionSemaphore = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL) );
    
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    strcpy((char *)wifi_config.sta.ssid, "eduroam");
    esp_wifi_sta_wpa2_ent_set_password( (unsigned char*)PASS, strlen(PASS) );
    esp_wifi_sta_wpa2_ent_set_username( (unsigned char*)"bdsouza@ualberta.ca" , strlen("bdsouza@ualberta.ca"));

    printf("WiFi configuring with:\n");
    printf("SSID: %s\n", wifi_config.sta.ssid);

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID)) );

    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
    ESP_ERROR_CHECK( esp_wifi_start() );

    xTaskCreate(&OnConnected, "Connection Handler", 1024*5, NULL, 5, NULL);  

    wififlag = 0;
}