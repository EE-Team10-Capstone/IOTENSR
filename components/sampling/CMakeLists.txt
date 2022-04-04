idf_component_register(SRCS "sampling.c" "sleep.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_hw_support
                    PRIV_REQUIRES i2c sensor thingspeak provisioning)
