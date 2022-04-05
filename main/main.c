#include "provisioning.h"
#include "sampling.h"
#include "sampling_main.h"
#include "i2c.h"
#include "microOLED.h"


RTC_DATA_ATTR SleepState sleepState;

void app_main(void)
{   
    // InitialDeepSleep(&sleepState);

    // prvsnState = InitialTry;
    // beginProvisioning();

    // while(!ProvisionTaskDone())
    // {
    //     printf("Provision Task in progress.\n");
    //     vTaskDelay(pdMS_TO_TICKS(5000));
    // }
    
    // beginSampling();

    initializeI2C();
    ssd1306_init();

    uint16_t co2=19; float temperature = 23.4; float humdity= 24;
	
	char CO2_ch[60] = ("CO2:");
	char *TEMP_ch = ("TEMP:");
	char *HUMID_ch = ("HUMID:");
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
    xTaskCreate(&task_ssd1306_display_text, "ssd1306_display_text",  2048,
		(void *)"Hello world!\nMulitine is OK!\nAnother line", 6, NULL);
	xTaskCreate(&task_ssd1306_contrast, "ssid1306_contrast", 2048, NULL, 6, NULL);
	// display_char(charer);


}