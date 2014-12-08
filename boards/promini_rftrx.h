/**
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#pragma once 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BOARD 
#error "Only one board file can be included in a project!"
#endif

#define BOARD

#include <avr/io.h>
#include <arch/m328p.h>
#include <radio/nrf24l01.h>
//#include <hid/joystick.h>

enum {
	A0, A1, A2, A3, A4, A5, A6, A7, 
	B0, B1, B2, B3, B4, B5, B6, B7, 
	C0, C1, C2, C3, C4, C5, C6, C7, 
	D0, D1, D2, D3, D4, D5, D6, D7
}; 

typedef enum {
	ANALOG_IN_0, 
	ANALOG_IN_1, 
	ANALOG_IN_2, 
	ANALOG_IN_3, 
	ANALOG_IN_4, 
	ANALOG_IN_5, 
	PWM_OUT_0, 
	PWM_OUT_1, 
	PWM_OUT_2, 
	PWM_OUT_3, 
	PWM_OUT_4, 
	PWM_OUT_5
} pin_t; 

/*
#define ANALOG_IN_0 	A0
#define ANALOG_IN_1 	A1
#define ANALOG_IN_2 	A2
#define ANALOG_IN_3 	A3
#define ANALOG_IN_4 	A6
#define ANALOG_IN_5 	A7

#define PWM_OUT_0 		D2
#define PWM_OUT_1 		D3
#define PWM_OUT_2 		D4
#define PWM_OUT_3			D5
#define PWM_OUT_4 		D6
#define PWM_OUT_5 		D7
*/
#define PACKET_MODEM0

struct modem {
	void (*send)(const uint8_t *data, uint16_t size); 
	uint16_t (*recv)(uint8_t *data); 
}; 

extern struct modem *modem0; 

void brd_init(void); 
void brd_process_events(void); 

void set_pin(uint8_t pin, uint16_t value);
uint16_t get_pin(uint8_t pin);

#ifdef __cplusplus
}
#endif
