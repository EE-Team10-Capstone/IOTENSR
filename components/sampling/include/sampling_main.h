typedef enum SleepStates {
    BOOTUP = 0,
    WAKE = 1
}SleepState;

void InitialDeepSleep(SleepState *sleepState);
