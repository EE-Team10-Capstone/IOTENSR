idf_component_register(SRCS "ble.c" "wifi.c" "provisioning.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos
                    PRIV_REQUIRES nvs_flash esp_wifi wpa_supplicant bt thingspeak)
