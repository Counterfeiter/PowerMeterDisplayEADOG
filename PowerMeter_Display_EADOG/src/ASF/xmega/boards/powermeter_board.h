/**
 * \file
 *
 * \brief STK600 with the RC044X routing card board header file.
 *
 * This file contains definitions and services related to the features of the
 * STK600 board with the routing card for 44-pin TQFP AVR XMEGA devices.
 *
 * To use this board, define BOARD=STK600_RC044X.
 *
 * Copyright (c) 2010-2011 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef _POWERMETER_BOARD_H_
#define _POWERMETER_BOARD_H_

#include "compiler.h"

//! @{

#define VUSB_I					IOPORT_CREATE_PIN(PORTD,4)
#define RS485_1_ACT_I			IOPORT_CREATE_PIN(PORTC,0)
#define RS485_2_ACT_I			IOPORT_CREATE_PIN(PORTC,1)
#define USER_SW_I				IOPORT_CREATE_PIN(PORTA,5)

#define LCD_DIM_O				IOPORT_CREATE_PIN(PORTE,3)
#define LCD_RS_O				IOPORT_CREATE_PIN(PORTD,5)
#define LCD_MOSI_O				IOPORT_CREATE_PIN(PORTD,3)
#define LCD_CLK_O				IOPORT_CREATE_PIN(PORTD,1)
#define LCD_CSB_O				IOPORT_CREATE_PIN(PORTD,0)

#define LCD_UART_SPI			USARTD0

#define DIR_RS485_1_O			IOPORT_CREATE_PIN(PORTC,4)

#define DIR_RS485_2_O			IOPORT_CREATE_PIN(PORTC,5)

#define RS485_1_UART			USARTC0
#define RS485_1_DMA_CHANNEL		0

#define RS485_2_UART			USARTC1
#define RS485_2_DMA_CHANNEL		1


#endif
