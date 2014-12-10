/*
l74hc4051 lib 0x01

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifndef L74HC4051_H_
#define L74HC4051_H_

#define L74HC4051_MAXCH 8

//setup ports
#define L74HC4051_DDR DDRD
#define L74HC4051_PORT PORTD
#define L74HC4051_PIN PIND
#define L74HC4051_S0PIN PD2
#define L74HC4051_S1PIN PD3
#define L74HC4051_S2PIN PD4

extern void l74hc4051_init(void);
extern void l74hc4051_setchannel(uint8_t channel);

#endif
