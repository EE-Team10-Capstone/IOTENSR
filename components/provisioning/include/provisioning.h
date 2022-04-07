#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

void beginProvisioning(void);
bool ProvisionTaskDone(void);

xSemaphoreHandle ProvisionTaskFlag;

typedef enum ProvisionStates {
    InitialTry,
    Retry
}provision_state;

provision_state prvsnState;