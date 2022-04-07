idf_component_register(SRCS "microOLED.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos esp_common
                    PRIV_REQUIRES log)
