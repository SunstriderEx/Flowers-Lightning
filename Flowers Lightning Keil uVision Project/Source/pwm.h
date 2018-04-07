#ifndef PWM_H_
#define PWM_H_

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

class PWM
{
public:
	static void Init();

	static void Enable();
	static void Disable();

	static void UpdateDuty(float duty); // [0; 1.0]

    // TIM_Period = 72000000 / 30000 - 1 = 2399 // 30 kHz
	static const int Period = 2399;

    static const float brightThreshold;
    static const float dimThreshold;
    
    static bool fadingModeIsActive;

private:
    static void InitLED();
};

#endif
