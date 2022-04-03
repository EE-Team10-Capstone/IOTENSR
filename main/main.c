#include "provisioning.h"
#include "sampling.h"
#include "microOLED.h"

void i2c_init_oled(){

    // Initialize ESP32 in Master Mode and initialize pins
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 100000
    };

     // Wait for SCD-4x initialization duration (1s)
    vTaskDelay(pdMS_TO_TICKS(3000));

	gpio_pad_select_gpio(12);
	gpio_set_direction(12, GPIO_MODE_OUTPUT);
	gpio_set_level(12, 1);
    // Install I2C Driver using above struct and I2C Port 0
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    printf("\nI2C Initialized!\n");

    // //Reset pin for oled active low
    // gpio_config_t io_conf = {};
    // //disable interrupt
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // //set as output mode
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // //bit mask of the pins that you want to set
    // io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // //disable pull-down mode
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // //disable pull-up mode
    // io_conf.pull_up_en = GPIO_PULLDOWN_ENABLE;
    // //configure GPIO with the given settings
    // gpio_config(&io_conf);

}

void app_main(void)
{   
    // prvsnState = InitialTry;
    // beginProvisioning();

    // while(!ProvisionTaskDone())
    // {
    //     printf("Provision Task in progress.\n");
    //     vTaskDelay(pdMS_TO_TICKS(5000));
    // }
    
    // beginSampling();

    i2c_init_oled();
    ssd1306_init();

    uint16_t co2=19; float temperature = 23.4; float humdity= 24;
	
	char CO2_ch[60] = ("\n\n\nCO2:");
	char *TEMP_ch = ("\n\nTEMP:");
	char *HUMID_ch = ("\n\n\nHUMID:");
	char buffer [sizeof(unsigned int)*8+1];
	sprintf(buffer, "%d", co2);
	char *charer = strcat(CO2_ch,buffer);
	strcat(CO2_ch,TEMP_ch);
	sprintf(buffer, "%2.1f", temperature);
	(void) strcat(CO2_ch,buffer);
	strcat(CO2_ch,HUMID_ch);
	sprintf(buffer, "%2.1f", humdity);
	(void) strcat(CO2_ch,buffer);
	puts(CO2_ch);
	xTaskCreate(&task_ssd1306_display_clear, "ssd1306_display_clear",  2048, NULL, 6, NULL);
	vTaskDelay(100/portTICK_PERIOD_MS);
	xTaskCreate(&task_ssd1306_contrast, "ssid1306_contrast", 2048, NULL, 6, NULL);
	display_char(charer);

}