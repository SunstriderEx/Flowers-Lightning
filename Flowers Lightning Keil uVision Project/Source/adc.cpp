#include "adc.h"

float ADC::photoresistorValueAvg; //int ADC::photoresistorValues[PhotoresistorValuesCount];

void ADC::Init()
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    // input of ADC (it doesn't seem to be needed, as default GPIO state is floating input)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;        // that's ADC1 (PA0 on STM32)
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
    //clock for ADC (max 14MHz --> 72/6=12MHz)
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    // enable ADC system clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
 
    // define ADC config
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  // we work in continuous sampling mode
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 0;
 
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0, 0, ADC_SampleTime_28Cycles5); // define regular conversion config
    ADC_Init(ADC1, &ADC_InitStructure);   //set config of ADC1
 
    // enable ADC
    ADC_Cmd(ADC1,ENABLE);  //enable ADC1
    
    //
    
    photoresistorValueAvg = 4095;
    /*for (int i = 0; i < PhotoresistorValuesCount; i++)
    {
        photoresistorValues[i] = 4095;
    }*/
}

void ADC::Calibrate()
{
    //  ADC calibration (optional, but recommended at power on)
    ADC_ResetCalibration(ADC1); // Reset previous calibration
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1); // Start new calibration (ADC must be off at that time)
    while(ADC_GetCalibrationStatus(ADC1));
}

void ADC::StartReading()
{
    // start conversion
    ADC_Cmd(ADC1,ENABLE);  //enable ADC1
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // start conversion (will be endless as we are in continuous mode)
}

void ADC::StopReading()
{
    ADC_Cmd(ADC1, DISABLE);
}

float ADC::GetPhotoresistorValue()
{
    unsigned int lastValue = ADC_GetConversionValue(ADC1);
    TIM3->PSC = lastValue * 8; // prescaler (Indicator LED)
    
    // https://electronix.ru/forum/lofiversion/index.php/t57634.html
    photoresistorValueAvg += (lastValue - photoresistorValueAvg) / 25000.0;
    // ешё как вариант - Алгоритм скользящего среднего
    
    return photoresistorValueAvg;
    
    /*for (int i = 0; i < PhotoresistorValuesCount - 1; i++)
    {
        photoresistorValues[i] = photoresistorValues[i + 1];
    }
    photoresistorValues[PhotoresistorValuesCount - 1] = lastValue;
    
    float averageValue = 0;
    for (int i = 0; i < PhotoresistorValuesCount; i++)
        averageValue += photoresistorValues[i];
    averageValue /= PhotoresistorValuesCount;
    
    return averageValue;*/
}
















