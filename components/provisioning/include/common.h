// Shared resources between ble.c and wifi.c

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "esp_wpa2.h"

xSemaphoreHandle connectionSemaphore;

wifi_config_t wifi_config;
bool ssid_provisioned;
bool user_provisioned;
bool pass_provisioned;
bool write_key_provisioned;
uint8_t wififlag;

bool network_is_alive();
void wifiInit();
void wifi_wpa2enterprise_initialize();

void ble_init();