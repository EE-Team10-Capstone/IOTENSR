#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SAMPLE_PERIOD_MS 60000*5 // 5 min sample period

uint64_t sleep_time;
uint64_t wake_time;

void initializeSleep(void);
void pushbuttonDebounce(void);
void GoToLightSleep(void);
void WakeUpLogic(void);