#ifndef RTC_H_
#define RTC_H_

#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} RTC_TimeTypeDef;

class RTCModule
{
public:
	static unsigned char Init();
    
    static void CounterToTime(uint32_t RTC_Counter, RTC_TimeTypeDef* RTC_TimeStruct);
    static uint32_t TimeToCounter(RTC_TimeTypeDef* RTC_TimeStruct);

    static const unsigned int CounterToResetRTC = 24 * 3600; // Reset RTC every day
    static void ResetRTC();

    static RTC_TimeTypeDef TimeToSleep;
    static RTC_TimeTypeDef TimeToAwake;
};

#endif
