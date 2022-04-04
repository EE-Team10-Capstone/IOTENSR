#include "provisioning.h"
#include "sampling.h"
#include "sampling_main.h"

RTC_DATA_ATTR SleepState sleepState;

void app_main(void)
{   
    InitialDeepSleep(&sleepState);

    prvsnState = InitialTry;
    beginProvisioning();

    while(!ProvisionTaskDone())
    {
        printf("Provision Task in progress.\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
    beginSampling();
}