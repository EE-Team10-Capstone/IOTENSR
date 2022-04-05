#include <stdio.h>
#include "i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_types.h"
#include "esp_log.h"
#include "driver/i2c.h"

void initializeI2C(void)
{
    // Initialize ESP32 in Master Mode and initialize pins
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 26,
        .scl_io_num = 25,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    // Install I2C Driver using above struct and I2C Port 0
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    printf("\nI2C Initialized!\n");

    // Wait for SCD-4x initialization duration (1s)
    vTaskDelay(pdMS_TO_TICKS(3000));
}
