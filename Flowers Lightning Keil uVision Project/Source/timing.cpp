#include "timing.h"

unsigned int Timing::millis = 0;

osTimerDef(millisCounter, Timing::IncrementTimerCounter);

void Timing::IncrementTimerCounter(void const *n) //extern "C" void HAL_SYSTICK_Callback() //SysTick_Handler()
{
    Timing::millis++;
}

void Timing::InitSysTickTimer()
{
    /*¬ поле HCLK_Frequency переменной RCC_ClocksTypeDef загружаетс€ значение текущей
    основной тактовой частоты, затем в функцию SysTick_Config передаетс€ количество тиков,
    которое должен сделать таймер SysTick до своего окончани€ счета.*/
    /*RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);*/
    osTimerId timer = osTimerCreate(osTimer(millisCounter), osTimerPeriodic, (void *)0);
    osTimerStart(timer, 1); // period = 1 ms
}

unsigned int Timing::Millis()
{
    return millis;
}

/*void delay_us(unsigned int time_us)
{
	// Load the delay period in microseconds
	SysTick->LOAD = (SystemCoreClock / 1000000) * time_us;

	// Clears the current value and the count flag
	SysTick->VAL = 0;
    
	// Waits until the count ends
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}*/


