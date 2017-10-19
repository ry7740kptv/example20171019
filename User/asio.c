#include<string.h>
#include "asio.h"
#include "delay.h"

/* BANKA */
#define PIN_CLK_SEL		GPIO_Pin_3
#define PIN_SPI_SS		GPIO_Pin_4
/* BANKB */
#define PIN_I2S_RST		GPIO_Pin_0
#define PIN_DIV_SEL0	GPIO_Pin_10
#define PIN_DIV_SEL1	GPIO_Pin_11

uint8_t asio_one_sample[ASIO_SMPSIZ];
uint8_t asio_zero_sample[] = {0, 0, 0, 0, 0, 0};

static struct{
	uint32_t rp;
	uint32_t wp;
	uint8_t  sample[ASIO_FIFO_SIZE][ASIO_SMPSIZ];
}ASIO_FIFO;


void asio_hwinit()
{
	GPIO_InitTypeDef GPIO_InitStt;
	SPI_InitTypeDef  SPI_InitStt;
	DMA_InitTypeDef  DMA_InitStt;
	NVIC_InitTypeDef NVIC_InitStt;
	
	/* enable clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | 
							RCC_APB2Periph_GPIOB | 
							RCC_APB2Periph_AFIO | 
							RCC_APB2Periph_SPI1, ENABLE);
	
	/* setup pins */
	GPIO_InitStt.GPIO_Pin = PIN_CLK_SEL;
	GPIO_InitStt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStt.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStt);
	
	GPIO_InitStt.GPIO_Pin = PIN_SPI_SS;
	GPIO_InitStt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStt.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStt);
	GPIO_SetBits(GPIOA, PIN_SPI_SS);
	
	GPIO_InitStt.GPIO_Pin = PIN_I2S_RST | PIN_DIV_SEL0 | PIN_DIV_SEL1;
	GPIO_InitStt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStt.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStt);
	GPIO_SetBits(GPIOB, PIN_I2S_RST);

	/* setup SPI */
	GPIO_InitStt.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStt.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStt.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStt);
	
	SPI_InitStt.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStt.SPI_Mode = SPI_Mode_Master;
	SPI_InitStt.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStt.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStt.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStt.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStt.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStt.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStt.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStt);
	
	DMA_InitStt.DMA_PeripheralBaseAddr = (uint32_t)(&SPI1->DR);
	DMA_InitStt.DMA_MemoryBaseAddr = (uint32_t)asio_one_sample;
	DMA_InitStt.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStt.DMA_BufferSize = ASIO_SMPSIZ;
	DMA_InitStt.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStt.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStt.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStt.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStt.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStt.DMA_Priority = DMA_Priority_High;
	DMA_InitStt.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStt);
	DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
	
	
	NVIC_InitStt.NVIC_IRQChannel = DMA1_Channel3_IRQn;
	NVIC_InitStt.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStt.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStt);
	
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_Cmd(SPI1, ENABLE);
	
	/* setup EXTI */
	GPIO_InitStt.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStt.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStt);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	
	NVIC_InitStt.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStt);
}

void asio_spiss(uint8_t act)
{
	if(act)
		GPIO_ResetBits(GPIOA, PIN_SPI_SS);
	else
		GPIO_SetBits(GPIOA, PIN_SPI_SS);
}

void asio_reset()
{
	GPIO_ResetBits(GPIOB, PIN_I2S_RST);
	delay_ms(1);
	GPIO_SetBits(GPIOB, PIN_I2S_RST);
}

void asio_samplerate(uint8_t rate)
{
	switch(rate){
		case 0: /* 44.1kHz */
			GPIO_SetBits(GPIOA, PIN_CLK_SEL);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL0);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL1);
			break;
		
		case 1: /* 48kHz */
			GPIO_ResetBits(GPIOA, PIN_CLK_SEL);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL0);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL1);
			break;
		
		case 2: /* 88.2kHz */
			GPIO_SetBits(GPIOA, PIN_CLK_SEL);
			GPIO_SetBits(GPIOB, PIN_DIV_SEL0);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL1);
			break;
		
		case 3: /* 96kHz */
			GPIO_ResetBits(GPIOA, PIN_CLK_SEL);
			GPIO_SetBits(GPIOB, PIN_DIV_SEL0);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL1);
			break;
		
		case 4: /* 176.4kHz */
			GPIO_SetBits(GPIOA, PIN_CLK_SEL);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL0);
			GPIO_SetBits(GPIOB, PIN_DIV_SEL1);
			break;
		
		case 5: /* 192kHz */
			GPIO_ResetBits(GPIOA, PIN_CLK_SEL);
			GPIO_ResetBits(GPIOB, PIN_DIV_SEL0);
			GPIO_SetBits(GPIOB, PIN_DIV_SEL1);
			break;
	}
}

void asio_transfer_control(uint8_t on)
{
	EXTI_InitTypeDef EXTI_InitStt;
	
	EXTI_InitStt.EXTI_Line = EXTI_Line2;
	EXTI_InitStt.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStt.EXTI_Trigger = EXTI_Trigger_Falling;
	if(on)
		EXTI_InitStt.EXTI_LineCmd = ENABLE;
	else
		EXTI_InitStt.EXTI_LineCmd = DISABLE;
	EXTI_ClearFlag(EXTI_Line2);
	EXTI_Init(&EXTI_InitStt);
}

void asio_fifo_init()
{
	ASIO_FIFO.rp = 0;
	ASIO_FIFO.wp = 0;
}

uint32_t asio_fifo_getnum()
{
	return (ASIO_FIFO.wp - ASIO_FIFO.rp + ASIO_FIFO_SIZE) % ASIO_FIFO_SIZE;
}

int asio_fifo_in(uint8_t *smp, uint16_t len)
{
	uint16_t i;
	
	for(i = 0; i < len / ASIO_SMPSIZ; i ++){
		if((ASIO_FIFO.wp + 1) % ASIO_FIFO_SIZE == ASIO_FIFO.rp){
			break;
		}
		
		memcpy(ASIO_FIFO.sample[ASIO_FIFO.wp], &smp[i * ASIO_SMPSIZ], ASIO_SMPSIZ);
		ASIO_FIFO.wp = (ASIO_FIFO.wp + 1) % ASIO_FIFO_SIZE;
	}
	
	return i? i : -1;
}

int asio_fifo_out(uint8_t *smp, uint8_t mute)
{
	if(ASIO_FIFO.rp == ASIO_FIFO.wp){
		memset(smp, 0, ASIO_SMPSIZ);
		return -1;
	}
	
	if(mute)
		memset(smp, 0, ASIO_SMPSIZ);
	else
		memcpy(smp, ASIO_FIFO.sample[ASIO_FIFO.rp], ASIO_SMPSIZ);
	ASIO_FIFO.rp = (ASIO_FIFO.rp + 1) % ASIO_FIFO_SIZE;
	
	return 0;
}
