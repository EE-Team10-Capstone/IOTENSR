#include "provisioning.h"
#include "sampling.h"
#include "i2c.h"
#include "microOLED.h"

RTC_DATA_ATTR SleepState sleepState;

void app_main(void)
{   
    InitialDeepSleep(&sleepState);

    initializeI2C();
    initializeOLED();

    printOLED(WAKE_UP_MSG);
    
    prvsnState = InitialTry;
    beginProvisioning();

    while(!ProvisionTaskDone())
    {
        printf("Provision Task in progress.\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
    beginSampling();
}