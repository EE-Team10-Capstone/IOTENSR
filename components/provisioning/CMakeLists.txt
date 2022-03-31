idf_component_register(SRCS "ble.c" "wifi.c" "provisioning.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos nvs_flash esp_wifi wpa_supplicant
                    PRIV_REQUIRES bt thingspeak)
