#include "rtc.h"

RTC_TimeTypeDef RTCModule::TimeToSleep =
{
    22, 30, 00
};

RTC_TimeTypeDef RTCModule::TimeToAwake =
{
    6, 30, 00
};

unsigned char RTCModule::Init()
{
    // Включить тактирование модулей управления питанием и управлением резервной областью
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Разрешить доступ к области резервных данных
    PWR_BackupAccessCmd(ENABLE);
    // Если RTC выключен - инициализировать
    if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
    {
        // Сброс данных в резервной области
        RCC_BackupResetCmd(ENABLE);
        RCC_BackupResetCmd(DISABLE);
 
        // Установить источник тактирования кварц 32768
        RCC_LSEConfig(RCC_LSE_ON);
        RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;
        while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY)
        {
            // visualization
            //for (int i = 0; i < 500000; i++) { }
            //GPIOC->ODR ^= GPIO_Pin_13;
        }
        
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
 
        RTC_SetPrescaler(0x7FFF); // Устанавливаем делитель, чтобы часы считали секунды
        
        // Включаем RTC
        RCC_RTCCLKCmd(ENABLE);
 
        // Ждем синхронизацию
        RTC_WaitForSynchro();
        
        return 1;
    }
    return 0;
}

// Get current time
void RTCModule::CounterToTime(uint32_t RTC_Counter, RTC_TimeTypeDef* RTC_TimeStruct)
{
    unsigned long time;
    unsigned long t1;
    int hour = 0;
    int min = 0;
    int sec = 0;
 
    time = RTC_Counter;
    t1 = time/60;
    sec = time - t1*60;
 
    time = t1;
    t1 = time/60;
    min = time - t1*60;
 
    time = t1;
    t1 = time/24;
    hour = time - t1*24;
 
    RTC_TimeStruct->hours = hour;
    RTC_TimeStruct->minutes = min;
    RTC_TimeStruct->seconds = sec;
}

// Convert Time to Counter
uint32_t RTCModule::TimeToCounter(RTC_TimeTypeDef* RTC_TimeStruct)
{
    uint32_t JDN = 0;

    JDN+=(RTC_TimeStruct->hours * 3600);
    JDN+=(RTC_TimeStruct->minutes * 60);
    JDN+=(RTC_TimeStruct->seconds);
 
    return JDN;
}

void RTCModule::ResetRTC()
{
    while (RTC_GetCounter() >= RTCModule::CounterToResetRTC)
    {
        RTC_SetCounter(RTC_GetCounter() - RTCModule::CounterToResetRTC);
    }
}


