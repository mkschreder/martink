/*
ledmatrix88 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <avr/io.h>

#include "ledmatrix88.h"

volatile uint8_t ledmatrix88_col = 0; //contains column data
volatile uint8_t ledmatrix88_row = 0; //contains row data

/*
 * get current col value
 */
uint8_t ledmatrix88_getcol(void) {
	return ledmatrix88_col;
}

/*
 * get current row value
 */
uint8_t ledmatrix88_gettrow(void) {
	return ledmatrix88_row;
}

/*
 * set col value
 */
void ledmatrix88_setcol(uint8_t col) {
	ledmatrix88_col = col;
}

/*
 * get row value
 */
void ledmatrix88_setrow(uint8_t row) {
	ledmatrix88_row = row;
}

/*
 * set ouput to zero
 */
void ledmatrix88_zero(void) {
	ledmatrix88_setcol(0);
	ledmatrix88_setrow(0);
}

/*
 * init ledmatrix
 */
void ledmatrix88_init() {
	//init col ports
	#if LEDMATRIX88_COLS >= 1
	LEDMATRIX88_COL1_DDR |= (1<<LEDMATRIX88_COL1_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 2
	LEDMATRIX88_COL2_DDR |= (1<<LEDMATRIX88_COL2_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 3
	LEDMATRIX88_COL3_DDR |= (1<<LEDMATRIX88_COL3_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 4
	LEDMATRIX88_COL4_DDR |= (1<<LEDMATRIX88_COL4_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 5
	LEDMATRIX88_COL5_DDR |= (1<<LEDMATRIX88_COL5_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 6
	LEDMATRIX88_COL6_DDR |= (1<<LEDMATRIX88_COL6_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 7
	LEDMATRIX88_COL7_DDR |= (1<<LEDMATRIX88_COL7_PINOUT); //output
	#endif
	#if LEDMATRIX88_COLS >= 8
	LEDMATRIX88_COL8_DDR |= (1<<LEDMATRIX88_COL8_PINOUT); //output
	#endif

	//init row ports
	#if LEDMATRIX88_ROWS >= 1
	LEDMATRIX88_ROW1_DDR |= (1<<LEDMATRIX88_ROW1_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 2
	LEDMATRIX88_ROW2_DDR |= (1<<LEDMATRIX88_ROW2_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 3
	LEDMATRIX88_ROW3_DDR |= (1<<LEDMATRIX88_ROW3_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 4
	LEDMATRIX88_ROW4_DDR |= (1<<LEDMATRIX88_ROW4_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 5
	LEDMATRIX88_ROW5_DDR |= (1<<LEDMATRIX88_ROW5_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 6
	LEDMATRIX88_ROW6_DDR |= (1<<LEDMATRIX88_ROW6_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 7
	LEDMATRIX88_ROW7_DDR |= (1<<LEDMATRIX88_ROW7_PINOUT); //output
	#endif
	#if LEDMATRIX88_ROWS >= 8
	LEDMATRIX88_ROW8_DDR |= (1<<LEDMATRIX88_ROW8_PINOUT); //output
	#endif
}

/*
 * output data
 */
void ledmatrix88_print() {
	//emit column data
	#if LEDMATRIX88_COLS >= 1
	if(ledmatrix88_col & (1<<0))
		LEDMATRIX88_COL1_PORT |= (1<<LEDMATRIX88_COL1_PINOUT); //on
	else
		LEDMATRIX88_COL1_PORT &= ~(1<<LEDMATRIX88_COL1_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 2
	if(ledmatrix88_col & (1<<1))
		LEDMATRIX88_COL2_PORT |= (1<<LEDMATRIX88_COL2_PINOUT); //on
	else
		LEDMATRIX88_COL2_PORT &= ~(1<<LEDMATRIX88_COL2_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 3
	if(ledmatrix88_col & (1<<2))
		LEDMATRIX88_COL3_PORT |= (1<<LEDMATRIX88_COL3_PINOUT); //on
	else
		LEDMATRIX88_COL3_PORT &= ~(1<<LEDMATRIX88_COL3_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 4
	if(ledmatrix88_col & (1<<3))
		LEDMATRIX88_COL4_PORT |= (1<<LEDMATRIX88_COL4_PINOUT); //on
	else
		LEDMATRIX88_COL4_PORT &= ~(1<<LEDMATRIX88_COL4_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 5
	if(ledmatrix88_col & (1<<4))
		LEDMATRIX88_COL5_PORT |= (1<<LEDMATRIX88_COL5_PINOUT); //on
	else
		LEDMATRIX88_COL5_PORT &= ~(1<<LEDMATRIX88_COL5_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 6
	if(ledmatrix88_col & (1<<5))
		LEDMATRIX88_COL6_PORT |= (1<<LEDMATRIX88_COL6_PINOUT); //on
	else
		LEDMATRIX88_COL6_PORT &= ~(1<<LEDMATRIX88_COL6_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 7
	if(ledmatrix88_col & (1<<6))
		LEDMATRIX88_COL7_PORT |= (1<<LEDMATRIX88_COL7_PINOUT); //on
	else
		LEDMATRIX88_COL7_PORT &= ~(1<<LEDMATRIX88_COL7_PINOUT); //off
	#endif
	#if LEDMATRIX88_COLS >= 8
	if(ledmatrix88_col & (1<<7))
		LEDMATRIX88_COL8_PORT |= (1<<LEDMATRIX88_COL8_PINOUT); //on
	else
		LEDMATRIX88_COL8_PORT &= ~(1<<LEDMATRIX88_COL8_PINOUT); //off
	#endif

	//emit row data
	#if LEDMATRIX88_ROWS >= 1
	if(ledmatrix88_row & (1<<0))
		LEDMATRIX88_ROW1_PORT |= (1<<LEDMATRIX88_ROW1_PINOUT); //on
	else
		LEDMATRIX88_ROW1_PORT &= ~(1<<LEDMATRIX88_ROW1_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 2
	if(ledmatrix88_row & (1<<1))
		LEDMATRIX88_ROW2_PORT |= (1<<LEDMATRIX88_ROW2_PINOUT); //on
	else
		LEDMATRIX88_ROW2_PORT &= ~(1<<LEDMATRIX88_ROW2_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 3
	if(ledmatrix88_row & (1<<2))
		LEDMATRIX88_ROW3_PORT |= (1<<LEDMATRIX88_ROW3_PINOUT); //on
	else
		LEDMATRIX88_ROW3_PORT &= ~(1<<LEDMATRIX88_ROW3_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 4
	if(ledmatrix88_row & (1<<3))
		LEDMATRIX88_ROW4_PORT |= (1<<LEDMATRIX88_ROW4_PINOUT); //on
	else
		LEDMATRIX88_ROW4_PORT &= ~(1<<LEDMATRIX88_ROW4_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 5
	if(ledmatrix88_row & (1<<4))
		LEDMATRIX88_ROW5_PORT |= (1<<LEDMATRIX88_ROW5_PINOUT); //on
	else
		LEDMATRIX88_ROW5_PORT &= ~(1<<LEDMATRIX88_ROW5_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 6
	if(ledmatrix88_row & (1<<5))
		LEDMATRIX88_ROW6_PORT |= (1<<LEDMATRIX88_ROW6_PINOUT); //on
	else
		LEDMATRIX88_ROW6_PORT &= ~(1<<LEDMATRIX88_ROW6_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 7
	if(ledmatrix88_row & (1<<6))
		LEDMATRIX88_ROW7_PORT |= (1<<LEDMATRIX88_ROW7_PINOUT); //on
	else
		LEDMATRIX88_ROW7_PORT &= ~(1<<LEDMATRIX88_ROW7_PINOUT); //off
	#endif
	#if LEDMATRIX88_ROWS >= 8
	if(ledmatrix88_row & (1<<7))
		LEDMATRIX88_ROW8_PORT |= (1<<LEDMATRIX88_ROW8_PINOUT); //on
	else
		LEDMATRIX88_ROW8_PORT &= ~(1<<LEDMATRIX88_ROW8_PINOUT); //off
	#endif
}

