#ifndef __ASIO_H
#define __ASIO_H

#include "stm32f10x.h"

#define ASIO_SMPSIZ		6
#define ASIO_FIFO_SIZE	320
#define ASIO_FIFO_UL	256
#define ASIO_FIFO_LL	64

extern uint8_t asio_one_sample[];
extern uint8_t asio_zero_sample[];

void	 asio_hwinit(void);
void	 asio_spiss(uint8_t);
void	 asio_reset(void);
void	 asio_samplerate(uint8_t);
void	 asio_transfer_control(uint8_t);
void	 asio_fifo_init(void);
uint32_t asio_fifo_getnum(void);
int		 asio_fifo_in(uint8_t *, uint16_t);
int		 asio_fifo_out(uint8_t *, uint8_t);

#endif
