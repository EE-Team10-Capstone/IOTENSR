idf_component_register(SRCS "sampling.c" "sleep.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES i2c sensor thingspeak)
