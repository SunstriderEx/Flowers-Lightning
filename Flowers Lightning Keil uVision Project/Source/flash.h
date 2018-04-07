#ifndef FLASH_H_
#define FLASH_H_

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"

#define SETTINGS_FLASH_PAGE_ADDR 0x800FC00

class Flash
{
public:
    static void Init();

    static void ReadSettings();

    static void WriteSettingsIfNeeded();
    static void StartOrUpdateSettingsWriteTimer();

private:
    typedef struct
    {
        uint32_t timeToSleepCounter; // 4 byte
        uint32_t timeToAwakeCounter; // 4 byte
        // 8 byte = 2  32-bits words.  It's - OK
        // !!! Full size (bytes) must be a multiple of 4 !!!
    } SettingsStruct;
    
    static SettingsStruct settings;
    
    static const uint8_t SettingsWords = sizeof(settings)/4;
    
    static void WriteSettings();
    
    static bool needToWrite;
    static unsigned int lastSettingsChangeTime;
    static const unsigned short SettingsWriteDelay = 1000; // ms
};

#endif
