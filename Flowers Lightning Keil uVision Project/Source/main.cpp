#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include "adc.h"
#include "pwm.h"
#include "rtc.h"
#include "supply.h"
#include "usart.h"
#include "timing.h"
#include "usb hid.h"
#include "flash.h"

#include <math.h>
#include <stdio.h>

void SetSysClockTo72()
{
    ErrorStatus HSEStartUpStatus;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();
 
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);
 
    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
 
    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        //FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);
 
        /* Flash 2 wait state */
        //FLASH_SetLatency( FLASH_Latency_2);
 
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
 
        /* PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1);
 
        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div1);
 
        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(0x00010000, RCC_PLLMul_9);
 
        /* Enable PLL */
        RCC_PLLCmd(ENABLE);
 
        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }
 
        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
 
        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock configuration.
     User can add here some code to deal with this error */
 
        /* Go to infinite loop */
        while (1)
        {
        }
    }
}

void Init()
{
    SetSysClockTo72();
    Timing::InitSysTickTimer();
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // Enable GPIOA clocking
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // Enable GPIOB clocking
    
    GPIO_InitTypeDef port;
    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_IPD;
    port.GPIO_Pin = GPIO_Pin_6;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &port);
}

float ldrValue;
float duty = 1;

int main()
{
    Init();

    // Initialize USART
    //USART::Init();
    //USART::USARTSend((const unsigned char*)"Hello.\r\nUSART1 is ready.\r\n");
    USB_HID::Init(); // нужно инициализировать до RTCModule::Init(), иначе задержка не даст копму получить дескриптор
                     // (при подключении по USB без батарейки в устройстве, вероятно)    
    Flash::Init();
    
    ADC::Init();
    ADC::Calibrate();
    ADC::StartReading();
    
    PWM::Init();
    PWM::Enable();
 
    unsigned int RTC_Counter;
    unsigned int RTC_Counter_TimeToSleep;
    unsigned int RTC_Counter_TimeToAwake;
    RTC_TimeTypeDef RTC_Time;
    if (RTCModule::Init() == 1)
    {
        // После инициализации требуется задержка. Без нее время не устанавливается.
        osDelay(100);//for (int i = 0; i < 1000000; i++) { } //delay_ms(500);
        // Если первая инициализация RTC устанавливаем начальное время
        RTC_Time.hours = 15;
        RTC_Time.minutes = 29;
        RTC_Time.seconds = 55;
        RTC_SetCounter(RTCModule::TimeToCounter(&RTC_Time));
    }
    // Считаем настройки времени из Flash памяти
    Flash::ReadSettings();
    
    // usart
    //char buffer[80] = {'\0'};
    unsigned int lastRTC_Counter = 0;
    
    while (1)
    {
        // Real Time Clock
        RTC_Counter = RTC_GetCounter();
        RTCModule::CounterToTime(RTC_Counter, &RTC_Time);
        // Reset RTC every day
        if (RTC_Counter >= RTCModule::CounterToResetRTC)
        {
            RTCModule::ResetRTC();
            RTC_Counter = RTC_GetCounter();
            RTCModule::CounterToTime(RTC_Counter, &RTC_Time);
        }
        
        // Sleeping Mode
        RTC_Counter_TimeToSleep = RTCModule::TimeToCounter((RTC_TimeTypeDef*)(&RTCModule::TimeToSleep));
        RTC_Counter_TimeToAwake = RTCModule::TimeToCounter((RTC_TimeTypeDef*)(&RTCModule::TimeToAwake));
        if ((RTC_Counter >= RTC_Counter_TimeToSleep && RTC_Counter < RTC_Counter_TimeToAwake && RTC_Counter_TimeToAwake > RTC_Counter_TimeToSleep)
            || ((RTC_Counter >= RTC_Counter_TimeToSleep || RTC_Counter < RTC_Counter_TimeToAwake) && RTC_Counter_TimeToAwake < RTC_Counter_TimeToSleep))
        {
            
            if (!Supply::SleepingModeIsActive())
            {
                PWM::fadingModeIsActive = true; // плавное затухание
                Supply::StartSleepingMode(false);
            }
            else if (Supply::SleepingModeIsForced()) // если в сон вошли кнопкой, то сбрасываем флаг forcedSleepingMode, 
                Supply::StartSleepingMode(false);    // чтобы [утром] автоматически выйти из режима сна (проснуться)
        }
        else
        {
            if (Supply::SleepingModeIsActive() && !Supply::SleepingModeIsForced())
                Supply::AwakeFromSleepingMode();
        }
        
        // Force Sleeping Mode button
        Supply::CheckSleepingModeButton();
        
        // PWM
        if (!Supply::SleepingModeIsActive())
        {
            ldrValue = ADC::GetPhotoresistorValue(); // LDR (Light Dependent Resistor) = Photoresistor
            duty = powf((ldrValue / (4096 - 1)), 0.5); //, 0.25);
            PWM::UpdateDuty(duty);
        }
        else if (PWM::fadingModeIsActive)
        {
            // плавное затухание
            duty += 0.0000005;
            PWM::UpdateDuty(duty);
            if (duty > PWM::dimThreshold)
                Supply::StartSleepingMode(true); // окончательный сон
        }
        
        // USART and USB
        //USB_HID::CheckUSBConnection();
        //sprintf(buffer, "ldrValue = %d, duty = %f\r\n", ldrValue, duty);
        if (Timing::Millis() - lastRTC_Counter >= 200) //if (lastRTC_Counter != RTC_Counter)
        {
            //sprintf(buffer, "counter = %d, Time = %d:%d:%d, millis = %d\r\n", RTC_Counter, RTC_Time.hours, RTC_Time.minutes, RTC_Time.seconds, Timing::Millis());
            //USART::USARTSend((unsigned char*)buffer);
            
            //sprintf(buffer, "%d:%d:%d", RTC_Time.hours, RTC_Time.minutes, RTC_Time.seconds);
            //USBD_HID_GetReportTrigger(0, USB_HID::DeviceRtcTime, (const uint8_t*)(&buffer), sizeof(buffer));
            USB_HID::SendSettings();
            
            lastRTC_Counter = Timing::Millis(); //RTC_Counter;
        }
        
        // Save settings if needed
        Flash::WriteSettingsIfNeeded();
    }
}





