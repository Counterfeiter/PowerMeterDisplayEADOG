// PowerMeter Display Extension
// Copyright (c) 2011-2015 Sebastian Förster
// delta-2000@gmx.de

#ifndef _LCD_EADOG_h_
#define _LCD_EADOG_h_

#include <asf.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_MENU	4

struct display_data {
	// handling
	uint8_t		menu_pos;
	uint8_t		measure; // 1 or 2 if 10 or 20 => disabled
	uint8_t		hall_active;
	
	//calculated values
	uint64_t	voltage_rms;
	int64_t		current_rms;
	uint32_t	voltage_mean;
	int32_t		current_mean;
	uint16_t	samples;
	
	//scale values
	float		scale_v;
	float		scale_c;
	
	//min max values
	uint16_t	min_v;
	uint16_t	max_v;
	int16_t		min_i;
	int16_t		max_i;
	int32_t		max_pow;
	int32_t		peak_pow;
};

extern uint8_t	lcd_eadog_create_menu(struct display_data *lcd_d);
extern void		lcd_eadog_reset_data(struct display_data *lcd_d, uint8_t measure);
extern void		lcd_eadog_set_cursor (unsigned char pos);
extern void		lcd_eadog_clear_home (void);
extern void		lcd_eadog_set_contrast (unsigned char con);
extern int		lcd_eadog_write_char (char car, FILE *unused);
extern void		lcd_eadog_reset_minmax(struct display_data *lcd_d);
extern void		lcd_eadog_init(void);

#endif
