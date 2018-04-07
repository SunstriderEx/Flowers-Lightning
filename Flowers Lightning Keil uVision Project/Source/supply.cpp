#include "supply.h"
#include "adc.h"
#include "pwm.h"
#include "timing.h"

bool Supply::sleepingMode = false;
bool Supply::forcedSleepingMode = false;

unsigned char Supply::forceSleepButtonState = 0;
unsigned char Supply::forceSleepButtonLastState = 0;
unsigned int Supply::forceSleepButtonLastPressTime = 0;
unsigned int Supply::forceSleepButtonLastReleaseTime = 0;
bool Supply::forceSleepButtonFired = false;

void Supply::StartSleepingMode(bool forced)
{
    sleepingMode = true;
    forcedSleepingMode = forced;
    
    if (!forcedSleepingMode)
        return;
    
    ADC::StopReading();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    PWM::Disable();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
    
    PWM::fadingModeIsActive = false;
}

void Supply::AwakeFromSleepingMode()
{
    if (PWM::fadingModeIsActive)
        StartSleepingMode(true);
    
    ADC::Init();
    ADC::Calibrate();
    ADC::StartReading();
    PWM::Init();
    PWM::Enable();
    
    sleepingMode = false;
    forcedSleepingMode = false;
}
	
bool Supply::SleepingModeIsActive()
{
    return sleepingMode;
}

bool Supply::SleepingModeIsForced()
{
    return forcedSleepingMode;
}

void Supply::CheckSleepingModeButton()
{
    forceSleepButtonState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);  
    if (forceSleepButtonState == 1)
    {   
        if (forceSleepButtonLastState == 0)
            forceSleepButtonLastPressTime = Timing::Millis();
        
        if (Timing::Millis() - forceSleepButtonLastPressTime >= 10 // ms
            && !forceSleepButtonFired)
        {
            forceSleepButtonFired = true;
            if (!Supply::SleepingModeIsActive())
                Supply::StartSleepingMode(true);
            else
                Supply::AwakeFromSleepingMode();
        }
    }
    else
    {
        if (forceSleepButtonLastState == 1)
            forceSleepButtonLastReleaseTime = Timing::Millis();
        
        if (Timing::Millis() - forceSleepButtonLastPressTime >= 200) // ms
            forceSleepButtonFired = false;
    }
    forceSleepButtonLastState = forceSleepButtonState;
}



