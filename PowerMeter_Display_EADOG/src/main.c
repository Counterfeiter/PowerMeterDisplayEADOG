/**
 *
 * Copyright (c) 2011-2014 Atmel Corporation. All rights reserved.
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

// PowerMeter Display Extension
// Copyright (c) 2011-2015 Sebastian Förster
// delta-2000@gmx.de

#include <asf.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "conf_usb.h"
#include "powermeter_board.h"
#include "protocol.h"
#include "rs485.h"
#include "LCD_EADOG.h"

void (*start_bootloader) (void) = (void (*)(void))(BOOT_SECTION_START/2+0x1FC/2);

struct eeprom_settings ee_scale_settings;
 
volatile uint8_t	timer_cnt=0;

volatile uint16_t	timer_button=0;

struct display_data lcd_data[2];


void TimerCallback(void);
void init_timer(void);

// Output-Stream
FILE lcd_str = FDEV_SETUP_STREAM(lcd_eadog_write_char, NULL, _FDEV_SETUP_WRITE);

void reset_to_bootloader(void)
{
	udc_stop();
	uint8_t temp_buf[EEPROM_PAGE_SIZE];
	temp_buf[0]=0xB0;
	nvm_eeprom_flush_buffer();
	nvm_eeprom_load_page_to_buffer(temp_buf);
	nvm_eeprom_atomic_write_page(EEPROM_PAGE_BOOT);
	delay_ms(500);
	wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
	wdt_enable();
}

void TimerCallback(void)
{
	timer_cnt++;
	if(gpio_pin_is_low(USER_SW_I) && timer_button < 10000)
		timer_button++;
	//gpio_toggle_pin(LED_GREEN_O);
}

void init_timer(void)
{

	tc_enable(&TCC0);

	tc_write_period(&TCC0,375*4);
	
	/*
	* Enable TC interrupts overflow
	*/
	tc_set_overflow_interrupt_callback(&TCC0,TimerCallback);
	tc_set_overflow_interrupt_level(&TCC0, TC_INT_LVL_LO);
	
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV8_gc);
	
	tc_enable(&TCE0);

	tc_write_period(&TCE0,375*2);
	
	tc_set_wgm(&TCE0,TC_WGMODE_SINGLESLOPE_gc);

	//TODO: let the user change the display background brightness
	tc_write_cc(&TCE0,TC_CCD, 200); //fixed value for the display brightness at the time
	tc_enable_cc_channels(&TCE0,TC_CCDEN);
	
	tc_write_clock_source(&TCE0, TC_CLKSEL_DIV64_gc);
}
int main(void)
{
	
	// stderr for fprintf
	stderr = &lcd_str;
		
	irq_initialize_vectors();
	cpu_irq_enable();

	// Initialize the sleep manager
	sleepmgr_init();

	sysclk_init();
	
	
	//should we start the bootloader???
	if(nvm_eeprom_read_byte(EEPROM_PAGE_BOOT * EEPROM_PAGE_SIZE)==0xB0) {
		uint8_t temp_buf[EEPROM_PAGE_SIZE];
		temp_buf[0]=0xFF;
		nvm_eeprom_flush_buffer();
		nvm_eeprom_load_page_to_buffer(temp_buf);
		nvm_eeprom_atomic_write_page(EEPROM_PAGE_BOOT);
		start_bootloader();
	}
		
	board_init();
	
	//always on
	rs485_init();
	
	init_timer();
	
	//reset but set not active
	lcd_eadog_reset_data(&lcd_data[0],10);
	lcd_eadog_reset_data(&lcd_data[1],20);
	
	//init display + set special chars
	lcd_eadog_init();
	
	lcd_eadog_set_cursor(0);
	fprintf(stderr,"PowerMeter");
	lcd_eadog_set_cursor(0x40);
	fprintf(stderr,"Display v%d.%d", POWER_METER_DISPLAY_VER1, POWER_METER_DISPLAY_VER2);
	
	delay_ms(700);
	
	lcd_eadog_set_cursor(0);
	fprintf(stderr,"waiting for data");
	lcd_eadog_set_cursor(0x40);
	fprintf(stderr,"...             ");
	
	// Start USB stack
	udc_start();
	
	//check if eeprom erased
	if(ee_scale_settings.opt_v == 0xFF) {
		
		nvm_eeprom_flush_buffer();
		nvm_eeprom_load_page_to_buffer((const uint8_t*)&ee_scale_settings);
		nvm_eeprom_atomic_write_page(EEPROM_PAGE_SCALE);
	}
	
	uint8_t menu_counter = 0;
	uint8_t display = 0;
	
	while (true) {
		
		//if power is missing of the port -> disable display data 
		if(gpio_pin_is_low(RS485_1_ACT_I)) {
			if(lcd_data[0].measure != 10) {
				if(lcd_data[1].measure != 20) {
					display = 2;
				} else {
					display = 0;
				}
				lcd_eadog_reset_data(&lcd_data[0],10);
			}
		}
		if(gpio_pin_is_low(RS485_2_ACT_I)) {
			if(lcd_data[1].measure != 20) {
				if(lcd_data[0].measure != 10) {
					display = 1;
				} else {
					display = 0;
				}
				lcd_eadog_reset_data(&lcd_data[1],20);
			}
		}
		
		if(display == 0) {
			if(lcd_data[0].measure == 1)
				display = 1;
			if(lcd_data[1].measure == 2)
				display = 2;
		}
		
		if(gpio_pin_is_high(USER_SW_I)) {
			cli();
			uint16_t temp_timer = timer_button;
			sei();
			if(temp_timer > 100 && temp_timer < 2000) {
				menu_counter++;
				if(menu_counter > MAX_MENU) {
					menu_counter = 0;
					if(display == 1) {
						if(lcd_data[1].measure != 20) {
							display = 2;
						}
					} else if(display == 2) {
						if(lcd_data[0].measure != 10) {
							display = 1;
						}
					}
				}
				lcd_data[0].menu_pos = menu_counter;
				lcd_data[1].menu_pos = menu_counter;
			} else if(temp_timer >= 2000 && temp_timer < 9000) {
				lcd_eadog_reset_minmax(&lcd_data[0]);
				lcd_eadog_reset_minmax(&lcd_data[1]);
				
				lcd_eadog_set_cursor(0);
				fprintf(stderr,"reset           ");
				lcd_eadog_set_cursor(0x40);
				fprintf(stderr,"...             ");
			} else if(temp_timer >= 9000) {
				reset_to_bootloader();
			}
			cli();
			timer_button = 0;
			sei();
		}
		
		//if usb active
		if((main_b_attached & 0x03) == 0x03) {
			//send data to display
			if(timer_cnt > 250) {
				timer_cnt = 0;
				if(display == 1)
					lcd_eadog_create_menu(&lcd_data[0]);
				else if(display == 2)
					lcd_eadog_create_menu(&lcd_data[1]);
					
				if(lcd_data[0].measure == 10 && lcd_data[1].measure == 20) 
				{					
					lcd_eadog_set_cursor(0);
					fprintf(stderr,"waiting for data");
					lcd_eadog_set_cursor(0x40);
					fprintf(stderr,"...             ");
				}
			}
			
			proto_receiver(&fifo_desc_1,&lcd_data[0]);
			proto_receiver(&fifo_desc_2,&lcd_data[1]);

		}
		
		//wenn sich attached status geändert hat
		if((main_b_attached & 0x01) && !(main_b_attached & 0x02)) {
			//entprellen
			delay_ms(1000);
			//ina aktivieren
			if(main_b_attached & 0x01) {
				main_b_attached = 0x03;
				
				delay_ms(500);
			}
		//wenn sich status zu dettached geändert hat
		} else if(!(main_b_attached & 0x01) && (main_b_attached & 0x02)) {
			//entprellen
			delay_ms(1000);

			if(!(main_b_attached & 0x01)) {
				main_b_attached = 0;
				//no real power save possible because there is no switch to disable RS485 for example
			}
		}

	}
}