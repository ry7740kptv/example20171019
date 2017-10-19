#include "stm32f10x.h"

void led_init()
{
	GPIO_InitTypeDef		GPIO_InitStt;
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStt;
	NVIC_InitTypeDef		NVIC_InitStt;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	GPIO_InitStt.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStt.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStt);
	
	TIM_TimeBaseStt.TIM_Prescaler = 36000 - 1;
	TIM_TimeBaseStt.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStt.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStt.TIM_Period = 1000 - 1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStt);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	NVIC_InitStt.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStt.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStt.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStt);
}

void led_on()
{
	TIM_Cmd(TIM2, DISABLE);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

void led_blink()
{
	TIM_Cmd(TIM2, ENABLE);
}

void led_off()
{
	TIM_Cmd(TIM2, DISABLE);
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}
