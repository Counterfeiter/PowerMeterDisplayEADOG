/*
 * protocol.h
 *
// PowerMeter Display Extension
// Copyright (c) 2011-2015 Sebastian Förster
// delta-2000@gmx.de
 */ 


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "main.h"
#include <asf.h>
#include "rs485.h"
#include "LCD_EADOG.h"


extern void proto_receiver(fifo_desc_t *fifo, struct display_data *lcd_data);


#endif /* PROTOCOL_H_ */