#include "flash.h"
#include "timing.h"
#include "rtc.h"

Flash::SettingsStruct Flash::settings;
bool Flash::needToWrite = false;
unsigned int Flash::lastSettingsChangeTime;

void Flash::Init()
{
    /* Next commands may be used in SysClock initialization function
       In this case using of FLASH_Init is not obligatorily */
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
}
 
void Flash::ReadSettings()
{
    //Read settings
    uint32_t *source_addr = (uint32_t *)SETTINGS_FLASH_PAGE_ADDR;
    uint32_t *dest_addr = (uint32_t *)&settings;
    for (uint16_t i = 0; i < SettingsWords; i++) {
        *dest_addr = *(__IO uint32_t*)source_addr;
        source_addr++;
        dest_addr++;
    }
    
    // проверим, не пустая ли память. В стертой памяти все биты установлены в "1"
    if (settings.timeToSleepCounter < 24 * 3600
        && settings.timeToAwakeCounter < 24 * 3600)
    {
        RTCModule::CounterToTime(settings.timeToSleepCounter, &RTCModule::TimeToSleep);
        RTCModule::CounterToTime(settings.timeToAwakeCounter, &RTCModule::TimeToAwake);
    }
}
 
void Flash::WriteSettings()
{
    FLASH_Unlock();
    FLASH_ErasePage(SETTINGS_FLASH_PAGE_ADDR);
 
    // Write settings
    uint32_t *source_addr = (uint32_t *)&settings;
    uint32_t *dest_addr = (uint32_t *)SETTINGS_FLASH_PAGE_ADDR;
    for (uint16_t i = 0; i < SettingsWords; i++) {
        FLASH_ProgramWord((uint32_t)dest_addr, *source_addr);
        source_addr++;
        dest_addr++;
    }
 
    FLASH_Lock();
}

void Flash::WriteSettingsIfNeeded()
{
    if (needToWrite
        && Timing::Millis() - lastSettingsChangeTime >= SettingsWriteDelay)
    {
        needToWrite = false;
        settings.timeToSleepCounter = RTCModule::TimeToCounter((RTC_TimeTypeDef*)(&RTCModule::TimeToSleep));
        settings.timeToAwakeCounter = RTCModule::TimeToCounter((RTC_TimeTypeDef*)(&RTCModule::TimeToAwake));
        WriteSettings();
    }
}

void Flash::StartOrUpdateSettingsWriteTimer()
{
    needToWrite = true;
    lastSettingsChangeTime = Timing::Millis();
}


