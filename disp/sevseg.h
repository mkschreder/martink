/*
sevseg lib 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef SEVSEG_H_
#define SEVSEG_H_

#include <avr/io.h>

//definitions
#define SEVSEG_TYPECC  0 //common cathode
#define SEVSEG_TYPECA  1 //common anode type
#define SEVSEG_TYPECCT 2 //common cathode / full transistor type
#define SEVSEG_TYPECAT 3 //common anode type / full transistor type

//set type
#define SEVSEG_TYPE SEVSEG_TYPECC

//set display pin, set on same port and ordered
#define SEVSEG_DDR DDRD
#define SEVSEG_PORT PORTD
#define SEVSEG_PINA PD0
#define SEVSEG_PINB PD1
#define SEVSEG_PINC PD2
#define SEVSEG_PIND PD3
#define SEVSEG_PINE PD4
#define SEVSEG_PINF PD5
#define SEVSEG_PING PD6
#define SEVSEG_PINDOT PD7

//set display power port
#define SEVSEG_DDRDISP DDRB
#define SEVSEG_PORTDISP PORTB
#define SEVSEG_PINDISP1 PB0
#define SEVSEG_PINDISP2 PB1

//set display array (of pin)
#define SEVSEG_PINDISPA {SEVSEG_PINDISP1, SEVSEG_PINDISP2}

//set minimum power on display time (us)
#define SEVSEG_USONTIME 1000

//functions
extern void sevseg_init(void);
extern void sevseg_setfirstdisplayactive(void);
extern void sevseg_putc(uint8_t c, uint8_t dot);
extern void sevseg_puthex(uint8_t h);
extern uint8_t sevseg_selnextdisplay(void);

#endif
