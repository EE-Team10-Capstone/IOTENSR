#include "provisioning.h"
#include "sampling.h"

void app_main(void)
{   
    prvsnState = InitialTry;

    beginProvisioning();

    while(!ProvisionTaskDone())
    {
        printf("Provision Task in progress.\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
    // ToDo: Semaphore for begin sample on app side
    beginSampling();




}