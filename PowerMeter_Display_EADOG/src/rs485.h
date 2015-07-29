/*
 * rs485.h
 
// PowerMeter Display Extension
// Copyright (c) 2011-2015 Sebastian Förster
// delta-2000@gmx.de
 */ 


#ifndef RS485_H_
#define RS485_H_


#include <asf.h>

#define FIFO_BUFFER_LENGTH		64

uint8_t rs485_buf_1[FIFO_BUFFER_LENGTH];
uint8_t rs485_buf_2[FIFO_BUFFER_LENGTH];

fifo_desc_t fifo_desc_1;
fifo_desc_t fifo_desc_2;


extern void rs485_init(void);
extern void rs485_deinit(void);
extern uint8_t rs485_add_data(uint8_t *data, uint8_t size);
extern void rs485_init_dma(void);


#endif /* RS485_H_ */