#ifndef ADC_H_
#define ADC_H_

#include "stm32f10x_adc.h"

class ADC
{
public:
	static void Init();

	static void Calibrate();

	static void StartReading();
    static void StopReading();

	static float GetPhotoresistorValue();

private:
    static float photoresistorValueAvg; //static int photoresistorValues[];
    //static const int PhotoresistorValuesCount = 3000; // RAM usage = RW-data=0 + ZI-data=9632; RAM limit = 20 Kb
};

#endif
