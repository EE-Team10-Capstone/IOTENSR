#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int64_t sleep_time;
int64_t wake_time;

void initializeSleep(void);
void pushbuttonDebounce(void);
void GoToLightSleep(void);
void GoToDeepSleep();
void WakeUpRoutine(void);