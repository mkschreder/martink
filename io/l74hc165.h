/*
l74hc165 lib 0x01

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <avr/io.h>

#ifndef L74HC165_H_
#define L74HC165_H_

//setup ports
#define L74HC165_DDR DDRD
#define L74HC165_PORT PORTD
#define L74HC165_PIN PIND
#define L74HC165_CLOCKPIN PD2
#define L74HC165_LOADPIN PD3
#define L74HC165_DATAPIN PD4

//setup number of chip attached to the board
#define L74HC165_ICNUMBER 2

extern void l74hc165_init(void);
extern void l74hc165_shiftin(uint8_t *bytearray);

#endif
