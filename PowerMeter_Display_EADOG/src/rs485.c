/*
 * rs485.c
 *
// PowerMeter Display Extension
// Copyright (c) 2011-2015 Sebastian Förster
// delta-2000@gmx.de
 */ 


#include "rs485.h"

ISR(USARTC0_RXC_vect) 
{
	fifo_push_uint8(&fifo_desc_1,USARTC0.DATA);
}

ISR(USARTC1_RXC_vect)
{
	fifo_push_uint8(&fifo_desc_2,USARTC1.DATA);
}

void rs485_init(void)
{
	if(fifo_init(&fifo_desc_1, rs485_buf_1, FIFO_BUFFER_LENGTH) != FIFO_OK) {
		return;
	}
	
	if(fifo_init(&fifo_desc_2, rs485_buf_2, FIFO_BUFFER_LENGTH) != FIFO_OK) {
		return;
	}
	
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 3), IOPORT_DIR_OUTPUT	| IOPORT_INIT_HIGH);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 2), IOPORT_DIR_INPUT);
	
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 7), IOPORT_DIR_OUTPUT	| IOPORT_INIT_HIGH);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 6), IOPORT_DIR_INPUT);
		
	//rs485 enable
	usart_rs232_options_t usart_opt;
	
	usart_opt.baudrate = 115200;
	usart_opt.charlength = USART_CHSIZE_8BIT_gc;
	usart_opt.paritytype = USART_PMODE_DISABLED_gc;
	usart_opt.stopbits = true;
	
	usart_init_rs232(&RS485_1_UART,&usart_opt);
	
	usart_set_rx_interrupt_level(&RS485_1_UART,USART_INT_LVL_HI);
	
	
	
	usart_init_rs232(&RS485_2_UART,&usart_opt);
	
	usart_set_rx_interrupt_level(&RS485_2_UART,USART_INT_LVL_HI);
}

void rs485_deinit(void)
{
	sysclk_disable_peripheral_clock(&RS485_1_UART);	
	sysclk_disable_peripheral_clock(&RS485_2_UART);
	
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 3), IOPORT_DIR_INPUT);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 2), IOPORT_DIR_INPUT);
	
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 7), IOPORT_DIR_INPUT);
	ioport_configure_pin(IOPORT_CREATE_PIN(PORTC, 6), IOPORT_DIR_INPUT);
	
	//dma_disable();
}