/*
 * protocol.c
 *
// PowerMeter Display Extension
// Copyright (c) 2011-2015 Sebastian Förster
// delta-2000@gmx.de
 */ 

#include "protocol.h"
#include <string.h>

#define TPM2_BLOCK_START			0xC9
#define TPM2_BLOCK_DATAFRAME		0xDA
#define TPM2_BLOCK_END				0x36

#define TPM2_ACK					0xAC

#define TPM2_NET_BLOCK_START		0x9C

#define PM_DATAFRAME				0xDA
#define PM_SCALEVALUES				0x5C
#define PM_SENDSCALE				0x5D
#define PM_HELLO					0x11
#define PM_START					0x5A
#define PM_STOP						0x50
#define PM_BOOTLOADER				0xB0


enum {IDLE, STARTB, STARTB2,STARTB3,STARTB4,STARTB5,STARTB6,ENDB};			//tpm2 states

static uint8_t tpm2state = IDLE;

//handle tpm2 protocol
static uint16_t count = 0;
static uint16_t Framesize = 0;

void proto_recv_data(uint8_t *buf, uint8_t len, struct display_data *lcd_data);
void proto_recv_scale(uint8_t *buf, uint8_t len, struct display_data *lcd_data);

void proto_recv_scale(uint8_t *buf, uint8_t len, struct display_data *lcd_data)
{
	if(len < 15) return;
	
	float *scale_V_t = (float*)&buf[2];
	float *scale_C_t = (float*)&buf[2 + 5];
	float *scale_CH_t = (float*)&buf[2 + 5 + 5];

	uint8_t hall_active_t = lcd_data->hall_active;

	if ((buf[10] & 0x04) > 0) {
		if (lcd_data->hall_active == false) {
			lcd_data->hall_active = true;
		}
		else {
			lcd_data->hall_active = true;
		}

		*scale_C_t = *scale_CH_t;

	}
	else {
		if (lcd_data->hall_active == true) {
			lcd_data->hall_active = false;
		}
		else {
			lcd_data->hall_active = false;
		}

		*scale_C_t = *scale_C_t;
	}
	
	//something has changed?
	if(lcd_data->scale_v != *scale_V_t || lcd_data->scale_c != *scale_C_t || lcd_data->hall_active != hall_active_t)
	{
		//set new values... reset the old buffers
		
		uint8_t mes = lcd_data->measure;
		if(mes == 20) {
			mes = 2;
		} else if (mes == 10) {
			mes = 1;
		}
		
		lcd_data->scale_v = *scale_V_t;
		lcd_data->scale_c = *scale_C_t;
		
		lcd_eadog_reset_data(lcd_data,mes);
	}
}

void proto_recv_data(uint8_t *buf, uint8_t len, struct display_data *lcd_data)
{
	//gpio_set_pin_high(LCD_CSB_O);
	for (uint8_t i = 1; i < len; i += 4)
	{
		uint16_t *point_v = (uint16_t*)&buf[i];
		int16_t  *point_i = (int16_t*)&buf[i+2];
		lcd_data->voltage_rms += (uint64_t)((uint32_t)(*point_v) * (*point_v));
		lcd_data->current_rms += (int64_t)((int32_t)(*point_i) * (*point_i));
		lcd_data->voltage_mean += (uint32_t)(*point_v);
		lcd_data->current_mean += (int32_t)(*point_i);
		lcd_data->samples++;
		
		int32_t temp_pow = (*point_v) * (*point_i);
		
		if(lcd_data->min_i > (*point_i)) lcd_data->min_i = (*point_i);
		if(lcd_data->max_i < (*point_i)) lcd_data->max_i = (*point_i);
		if(lcd_data->min_v > (*point_v)) lcd_data->min_v = (*point_v);
		if(lcd_data->max_v < (*point_v)) lcd_data->max_v = (*point_v);
		
		//record is zero
		if(lcd_data->peak_pow == 0) {
			lcd_data->peak_pow = temp_pow;
		//record is negative
		} else if(lcd_data->peak_pow < 0) {
			if(lcd_data->peak_pow > temp_pow) {
				lcd_data->peak_pow = temp_pow;
			}
			if(temp_pow > 0) {
				if(lcd_data->peak_pow * (-1) < temp_pow) {
					lcd_data->peak_pow = temp_pow;
				}
			}
		//record is positive
		} else {
			if(lcd_data->peak_pow < temp_pow) {
				lcd_data->peak_pow = temp_pow;
			}
			if(temp_pow < 0) {
				if(lcd_data->peak_pow * (-1) > temp_pow) {
					lcd_data->peak_pow = temp_pow;
				}
			}
			
		}
	}
	//gpio_set_pin_low(LCD_CSB_O);
}


uint8_t msg_buf[64];
uint8_t byte;
	
void proto_receiver(fifo_desc_t *fifo, struct display_data *lcd_data)
{

	
	cli();
	while(fifo_pull_uint8(fifo,&byte) == FIFO_OK) {
		sei();
		if(tpm2state == STARTB4) {
			if(count < Framesize) {
				msg_buf[count] = byte;
				count++;
				continue;
			}
			else
			tpm2state = ENDB;
		}

		//check for start- and sizebyte
		if(tpm2state == IDLE && byte == TPM2_BLOCK_START) {
			tpm2state = STARTB;
			continue;
		}
		
		if(tpm2state == STARTB && byte == TPM2_BLOCK_DATAFRAME) {
			tpm2state = STARTB2;
			continue;
		}
		
		if(tpm2state == STARTB2)
		{
			Framesize = (uint16_t)byte<<8;
			tpm2state = STARTB3;
			continue;
		}
		
		if(tpm2state == STARTB3)
		{
			Framesize |= (uint16_t)byte;
			if(Framesize <= 60) {
				count = 0;
				tpm2state = STARTB4;
			} else tpm2state = IDLE;
			
			continue;
			
		}

		//check end byte
		if(tpm2state == ENDB) {
			if(byte == TPM2_BLOCK_END) {
				switch(msg_buf[0]) {
					case PM_DATAFRAME:
					{
						proto_recv_data(msg_buf,count,lcd_data);
					}
					break;
					case PM_SCALEVALUES:
					{
						proto_recv_scale(msg_buf,count,lcd_data);
					}
					break;
					case PM_HELLO:
					{
						nvm_eeprom_flush_buffer();
					}
					break;
					case PM_BOOTLOADER:
					{
						reset_to_bootloader();
					}
					break;
					default:
					{
						nvm_eeprom_flush_buffer();
					}
					break;
				}
			}

			tpm2state = IDLE;
		}
	}
	sei();
}