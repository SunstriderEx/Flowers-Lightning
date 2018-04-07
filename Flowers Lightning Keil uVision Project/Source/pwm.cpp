#include "pwm.h"

const float PWM::brightThreshold = 0.001;
const float PWM::dimThreshold = 0.999;
bool PWM::fadingModeIsActive;

extern "C" void TIM3_IRQHandler()
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        GPIOC->ODR ^= GPIO_Pin_13;
    }
}

void PWM::Init()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  // Enable TIM4 clocking
    
    GPIO_InitTypeDef port;
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef timerPWM;
 
    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_AF_PP;
    port.GPIO_Pin = GPIO_Pin_7; // pin PB7
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &port);
 
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = 0;
    timer.TIM_Period = Period;
    timer.TIM_ClockDivision = 0; //TIM_CKD_DIV1;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &timer);
 
    TIM_OCStructInit(&timerPWM);
    // PWM mode 2 = Clear on compare match
    // PWM mode 1 = Set on compare match
    timerPWM.TIM_OCMode = TIM_OCMode_PWM2;
    timerPWM.TIM_OutputState = TIM_OutputState_Enable;
    timerPWM.TIM_Pulse = Period - 1;
    timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM4, &timerPWM);
    
    //UpdateDuty(1);
    
    InitLED();
}

void PWM::Enable()
{
    GPIO_SetBits(GPIOC, GPIO_Pin_13); // Off LED
    TIM_Cmd(TIM4, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void PWM::Disable()
{
    TIM_Cmd(TIM4, DISABLE);
    TIM_Cmd(TIM3, DISABLE);
    UpdateDuty(1);
    GPIO_SetBits(GPIOC, GPIO_Pin_13); // Off LED
}

void PWM::UpdateDuty(float duty)
{
    /*To get proper duty cycle, you have simple equation
    
    pulse_length = ((TIM_Period + 1) * DutyCycle) / 100 - 1
    
    where DutyCycle is in percent, between 0 and 100%
    
    25% duty cycle:     pulse_length = ((8399 + 1) * 25) / 100 - 1 = 2099
    50% duty cycle:     pulse_length = ((8399 + 1) * 50) / 100 - 1 = 4199
    75% duty cycle:     pulse_length = ((8399 + 1) * 75) / 100 - 1 = 6299
    100% duty cycle:    pulse_length = ((8399 + 1) * 100) / 100 - 1 = 8399
    
    Remember: if pulse_length is larger than TIM_Period, you will have output HIGH all the time*/
    
    if (duty > dimThreshold)
        duty = 1.0;
    else if (duty < brightThreshold)
        duty = 0;
    
    duty = (Period + 1) * duty - 1;
    
    TIM4->CCR2 = duty;

    //TIM3->PSC = duty * 10 + 1000; // prescaler (Indicator LED)
}

void PWM::InitLED()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  // Enable TIM3 clocking
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // Enable PORTC Clock
    
    // Initialize LED which connected to PC13
    GPIO_InitTypeDef  GPIO_InitStructure;
    // Configure the GPIO_LED pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
 
    GPIO_SetBits(GPIOC, GPIO_Pin_13); // Off LED
    
    //
    TIM_TimeBaseInitTypeDef timer;
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = 10000;
    timer.TIM_Period = Period;
    timer.TIM_ClockDivision = 0;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &timer);
 
    TIM_OCInitTypeDef timerOC;
    TIM_OCStructInit(&timerOC);
    timerOC.TIM_OCMode = TIM_OCMode_PWM2;
    timerOC.TIM_OutputState = TIM_OutputState_Enable;
    timerOC.TIM_Pulse = 50;
    timerOC.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &timerOC);
    
    // NVIC Configuration
    // Enable the TIM3_IRQn Interrupt
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}




