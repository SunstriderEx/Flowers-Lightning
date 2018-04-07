#ifndef TIMING_H_
#define TIMING_H_

#include "stm32f10x_rcc.h"
#include "cmsis_os.h"

class Timing
{
public:
    static void InitSysTickTimer();
    static void IncrementTimerCounter(void const *n);

    static unsigned int Millis();
    static unsigned int millis; // 0 to 4,294,967,295
};

#endif
