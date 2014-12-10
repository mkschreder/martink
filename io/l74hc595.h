#pragma once 

//setup ports
#define L74HC595_DDR DDRB
#define L74HC595_PORT PORTB
#define L74HC595_STCPIN PB1
//#define L74HC595_CEPIN PB2

void l74hc595_init(void);
void l74hc595_write(uint8_t val);
 
