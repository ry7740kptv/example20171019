#include "stm32f10x.h"

void delay_us(uint16_t us)
{
	uint32_t tmp;
	
	SysTick->LOAD = 9 * us;
	SysTick->VAL = 0x00000000;
	SysTick->CTRL = 0x00000001;
	
	do
		tmp = SysTick->CTRL;
	while((tmp & 0x00000001) && !(tmp & 0x00010000));
	
	SysTick->CTRL = 0x00000000;
	SysTick->VAL = 0x00000000;
}

void delay_ms(uint16_t ms)
{
	uint32_t tmp;
	
	SysTick->LOAD = 9000 * ms;
	SysTick->VAL = 0x00000000;
	SysTick->CTRL = 0x00000001;
	
	do
		tmp = SysTick->CTRL;
	while((tmp & 0x00000001) && !(tmp & 0x00010000));
	
	SysTick->CTRL = 0x00000000;
	SysTick->VAL = 0x00000000;
}
