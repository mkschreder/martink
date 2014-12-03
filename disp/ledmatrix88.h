/*
ledmatrix88 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef LEDMATRIX88_H_
#define LEDMATRIX88_H_

//define led matrix columns and rows
#define LEDMATRIX88_COLS 8
#define LEDMATRIX88_ROWS 8

//setup led matrix columns port
#if LEDMATRIX88_COLS >= 1
#define LEDMATRIX88_COL1_DDR DDRC
#define LEDMATRIX88_COL1_PORT PORTC
#define LEDMATRIX88_COL1_PINOUT PC0
#endif
#if LEDMATRIX88_COLS >= 2
#define LEDMATRIX88_COL2_DDR DDRC
#define LEDMATRIX88_COL2_PORT PORTC
#define LEDMATRIX88_COL2_PINOUT PC1
#endif
#if LEDMATRIX88_COLS >= 3
#define LEDMATRIX88_COL3_DDR DDRC
#define LEDMATRIX88_COL3_PORT PORTC
#define LEDMATRIX88_COL3_PINOUT PC2
#endif
#if LEDMATRIX88_COLS >= 4
#define LEDMATRIX88_COL4_DDR DDRC
#define LEDMATRIX88_COL4_PORT PORTC
#define LEDMATRIX88_COL4_PINOUT PC3
#endif
#if LEDMATRIX88_COLS >= 5
#define LEDMATRIX88_COL5_DDR DDRC
#define LEDMATRIX88_COL5_PORT PORTC
#define LEDMATRIX88_COL5_PINOUT PC4
#endif
#if LEDMATRIX88_COLS >= 6
#define LEDMATRIX88_COL6_DDR DDRD
#define LEDMATRIX88_COL6_PORT PORTD
#define LEDMATRIX88_COL6_PINOUT PC5
#endif
#if LEDMATRIX88_COLS >= 7
#define LEDMATRIX88_COL7_DDR DDRB
#define LEDMATRIX88_COL7_PORT PORTB
#define LEDMATRIX88_COL7_PINOUT PB1
#endif
#if LEDMATRIX88_COLS >= 8
#define LEDMATRIX88_COL8_DDR DDRB
#define LEDMATRIX88_COL8_PORT PORTB
#define LEDMATRIX88_COL8_PINOUT PB2
#endif

//setup led matrix rows port
#if LEDMATRIX88_ROWS >= 1
#define LEDMATRIX88_ROW1_DDR DDRD
#define LEDMATRIX88_ROW1_PORT PORTD
#define LEDMATRIX88_ROW1_PINOUT PD0
#endif
#if LEDMATRIX88_ROWS >= 2
#define LEDMATRIX88_ROW2_DDR DDRD
#define LEDMATRIX88_ROW2_PORT PORTD
#define LEDMATRIX88_ROW2_PINOUT PD2
#endif
#if LEDMATRIX88_ROWS >= 3
#define LEDMATRIX88_ROW3_DDR DDRD
#define LEDMATRIX88_ROW3_PORT PORTD
#define LEDMATRIX88_ROW3_PINOUT PD3
#endif
#if LEDMATRIX88_ROWS >= 4
#define LEDMATRIX88_ROW4_DDR DDRD
#define LEDMATRIX88_ROW4_PORT PORTD
#define LEDMATRIX88_ROW4_PINOUT PD4
#endif
#if LEDMATRIX88_ROWS >= 5
#define LEDMATRIX88_ROW5_DDR DDRD
#define LEDMATRIX88_ROW5_PORT PORTD
#define LEDMATRIX88_ROW5_PINOUT PD5
#endif
#if LEDMATRIX88_ROWS >= 6
#define LEDMATRIX88_ROW6_DDR DDRD
#define LEDMATRIX88_ROW6_PORT PORTD
#define LEDMATRIX88_ROW6_PINOUT PD6
#endif
#if LEDMATRIX88_ROWS >= 7
#define LEDMATRIX88_ROW7_DDR DDRD
#define LEDMATRIX88_ROW7_PORT PORTD
#define LEDMATRIX88_ROW7_PINOUT PD7
#endif
#if LEDMATRIX88_ROWS >= 8
#define LEDMATRIX88_ROW8_DDR DDRB
#define LEDMATRIX88_ROW8_PORT PORTB
#define LEDMATRIX88_ROW8_PINOUT PB0
#endif

//functions
extern void ledmatrix88_init(void);
extern uint8_t ledmatrix88_getcol(void);
extern uint8_t ledmatrix88_getrow(void);
extern void ledmatrix88_setcol(uint8_t col);
extern void ledmatrix88_setrow(uint8_t row);
extern void ledmatrix88_zero(void);
extern void ledmatrix88_print(void);

#endif
