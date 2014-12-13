/*************************************************************************
Title:    Interrupt UART library with receive/transmit circular buffers
Author:   Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:     $Id: uart.c,v 1.6.2.2 2009/11/29 08:56:12 Peter Exp $
Software: AVR-GCC 4.1, AVR Libc 1.4.6 or higher
Hardware: any AVR with built-in UART, 
License:  GNU General Public License 
          
DESCRIPTION:
    An interrupt is generated when the UART has finished transmitting or
    receiving a byte. The interrupt handling routines use circular buffers
    for buffering received and transmitted data.
    
    The UART_RX_BUFFER_SIZE and UART_TX_BUFFER_SIZE variables define
    the buffer size in bytes. Note that these variables must be a 
    power of 2.
    
USAGE:
    Refere to the header file uart.h for a description of the routines. 
    See also example test_uart.c.

NOTES:
    Based on Atmel Application Note AVR306
                    
LICENSE:
    Copyright (C) 2006 Peter Fleury

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
                        
*************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <arch/soc.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <static_cbuf.h>

#define SIG_USART_RECV USART_RX_vect
#define SIG_USART_DATA USART_UDRE_vect

/*
 *  constants and macros
 */

/* size of RX/TX buffers */
#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1)

#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif

#if defined(__AVR_AT90S2313__) \
 || defined(__AVR_AT90S4414__) || defined(__AVR_AT90S4434__) \
 || defined(__AVR_AT90S8515__) || defined(__AVR_AT90S8535__) \
 || defined(__AVR_ATmega103__)
 /* old AVR classic or ATmega103 with one UART */
 #define AT90_UART
 #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
 #define UART0_STATUS   USR
 #define UART0_CONTROL  UCR
 #define UART0_DATA     UDR  
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_AT90S2333__) || defined(__AVR_AT90S4433__)
 /* old AVR classic with one UART */
 #define AT90_UART
 #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR 
 #define UART0_UDRIE    UDRIE
#elif  defined(__AVR_ATmega8__)  || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) \
  || defined(__AVR_ATmega8515__) || defined(__AVR_ATmega8535__) \
  || defined(__AVR_ATmega323__)
  /* ATmega with one USART */
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega163__) 
  /* ATmega163 with one UART */
 #define ATMEGA_UART
 #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega162__) 
 /* ATmega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_RECEIVE_INTERRUPT   SIG_USART0_RECV
 #define UART1_RECEIVE_INTERRUPT   SIG_USART1_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART0_DATA
 #define UART1_TRANSMIT_INTERRUPT  SIG_USART1_DATA
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS   UCSR1A
 #define UART1_CONTROL  UCSR1B
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) 
 /* ATmega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_RECEIVE_INTERRUPT   SIG_UART0_RECV
 #define UART1_RECEIVE_INTERRUPT   SIG_UART1_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_UART0_DATA
 #define UART1_TRANSMIT_INTERRUPT  SIG_UART1_DATA
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS   UCSR1A
 #define UART1_CONTROL  UCSR1B
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega161__)
 /* ATmega with UART */
 #error "AVR ATmega161 currently not supported by this libaray !"
#elif defined(__AVR_ATmega169__) 
 /* ATmega with one USART */
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   SIG_USART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART_DATA
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)
 /* ATmega with one USART */
 #define ATMEGA_USART0
 #define UART0_RECEIVE_INTERRUPT   SIG_USART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART_DATA
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATtiny2313__)
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   SIG_USART0_RX 
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART0_UDRE
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega329__) ||defined(__AVR_ATmega3290__) ||\
      defined(__AVR_ATmega649__) ||defined(__AVR_ATmega6490__) ||\
      defined(__AVR_ATmega325__) ||defined(__AVR_ATmega3250__) ||\
      defined(__AVR_ATmega645__) ||defined(__AVR_ATmega6450__)
  /* ATmega with one USART */
  #define ATMEGA_USART0
  #define UART0_RECEIVE_INTERRUPT   SIG_UART_RECV
  #define UART0_TRANSMIT_INTERRUPT  SIG_UART_DATA
  #define UART0_STATUS   UCSR0A
  #define UART0_CONTROL  UCSR0B
  #define UART0_DATA     UDR0
  #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega1280__)  || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega640__)
/* ATmega with two USART */
  #define ATMEGA_USART0
  #define ATMEGA_USART1
  #define UART0_RECEIVE_INTERRUPT   SIG_USART0_RECV
  #define UART1_RECEIVE_INTERRUPT   SIG_USART1_RECV
  #define UART0_TRANSMIT_INTERRUPT  SIG_USART0_DATA
  #define UART1_TRANSMIT_INTERRUPT  SIG_USART1_DATA
  #define UART0_STATUS   UCSR0A
  #define UART0_CONTROL  UCSR0B
  #define UART0_DATA     UDR0
  #define UART0_UDRIE    UDRIE0
  #define UART1_STATUS   UCSR1A
  #define UART1_CONTROL  UCSR1B
  #define UART1_DATA     UDR1
  #define UART1_UDRIE    UDRIE1  
#elif defined(__AVR_ATmega644__)
 /* ATmega with one USART */
 #define ATMEGA_USART0
 #define UART0_RECEIVE_INTERRUPT   SIG_USART_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART_DATA
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__)
 /* ATmega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_RECEIVE_INTERRUPT   SIG_USART_RECV
 #define UART1_RECEIVE_INTERRUPT   SIG_USART1_RECV
 #define UART0_TRANSMIT_INTERRUPT  SIG_USART_DATA
 #define UART1_TRANSMIT_INTERRUPT  SIG_USART1_DATA
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS   UCSR1A
 #define UART1_CONTROL  UCSR1B
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#else
 #error "no UART definition for MCU available"
#endif

DECLARE_STATIC_CBUF(uart0_tx_buf, char, UART_TX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart0_rx_buf, char, UART_RX_BUFFER_SIZE);

static struct {
	uint8_t error;
} uart0;

/*
#ifdef CONFIG_HAVE_UART0
static volatile struct uart0 {
	volatile unsigned char UART_TxBuf[UART_TX_BUFFER_SIZE];
	volatile unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
	volatile int16_t UART_TxHead;
	volatile int16_t UART_TxTail;
	volatile int16_t UART_RxHead;
	volatile int16_t UART_RxTail;
	volatile unsigned char UART_LastRxError;
} _uart0;
static volatile struct uart0 *uart0 = &_uart0;
#endif

#ifdef CONFIG_HAVE_UART1
static volatile struct uart1 {
	volatile unsigned char TxBuf[UART1_TX_BUFFER_SIZE];
	volatile unsigned char RxBuf[UART1_RX_BUFFER_SIZE];
	volatile int16_t TxHead;
	volatile int16_t TxTail;
	volatile int16_t RxHead;
	volatile int16_t RxTail;
	volatile unsigned char LastRxError;
} _uart1;
static volatile struct uart1 *uart1 = &_uart1;
#endif
*/

ISR(UART0_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART Receive Complete interrupt
Purpose:  called when the UART has received a character
**************************************************************************/
{
	uint8_t err = ( UART0_STATUS & (_BV(FE0)|_BV(DOR0)));
	uint8_t data = UDR0;  
	if(cbuf_is_full(&uart0_rx_buf)){
		err = UART_BUFFER_OVERFLOW >> 8;
	} else {
		cbuf_put(&uart0_rx_buf, data);
	}
	uart0.error = err; 
	/*
    unsigned char tmphead;
    unsigned char data;
    unsigned char usr;
    unsigned char lastRxError;
 
 
    usr  = UART0_STATUS;
    data = UART0_DATA;
    
#if defined( AT90_UART )
    lastRxError = (usr & (_BV(FE)|_BV(DOR)) );
#elif defined( ATMEGA_USART )
    lastRxError = (usr & (_BV(FE)|_BV(DOR)) );
#elif defined( ATMEGA_USART0 )
    lastRxError = (usr & (_BV(FE0)|_BV(DOR0)) );
#elif defined ( ATMEGA_UART )
    lastRxError = (usr & (_BV(FE)|_BV(DOR)) );
#endif
        
    tmphead = ( uart0->UART_RxHead + 1) & UART_RX_BUFFER_MASK;
    
    if ( tmphead == uart0->UART_RxTail ) {
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        uart0->UART_RxHead = tmphead;
        uart0->UART_RxBuf[tmphead] = data;
    }
    uart0->UART_LastRxError = lastRxError;
    */
}

/*************************************************************************
Function: UART Data Register Empty interrupt
Purpose:  called when the UART is ready to transmit the next byte
**************************************************************************/
ISR(UART0_TRANSMIT_INTERRUPT)
{
	if(cbuf_get_data_count(&uart0_tx_buf)){
		// send one byte over the wire
		UDR0 = cbuf_get(&uart0_tx_buf);
	} else {
		// disable interrupt
		uart0_interrupt_dre_off(); 
	}
	/*
	unsigned char tmptail;
	if ( uart0->UART_TxHead != uart0->UART_TxTail) {
		tmptail = (uart0->UART_TxTail + 1) & UART_TX_BUFFER_MASK;
		uart0->UART_TxTail = tmptail;
		UART0_DATA = uart0->UART_TxBuf[tmptail];  
	} else {
		UART0_CONTROL &= ~_BV(UART0_UDRIE);
	}*/
}

/*************************************************************************
Function: uart_init()
Purpose:  initialize UART and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void PFDECL(CONFIG_UART0_NAME, init, unsigned int baudrate)
{
	uart0_init_default(baudrate); 

	uart0_interrupt_rx_on(); 

	UCSR0C = (3<<UCSZ00);
}

uint16_t PFDECL(CONFIG_UART0_NAME, waiting, void){
	uart0_interrupt_rx_off(); 
	int wait = cbuf_get_data_count(&uart0_rx_buf);
	uart0_interrupt_rx_on();
	return wait; 
	/*
	int16_t l = (int16_t)uart0->UART_RxHead - (int16_t)uart0->UART_RxTail; 
	return (l < 0)?(l+UART_RX_BUFFER_SIZE):l;*/
}

/*************************************************************************
Function: uart_getc()
Purpose:  return byte from ringbuffer  
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
unsigned int PFDECL(CONFIG_UART0_NAME, getc, void)
{
	uart0_interrupt_rx_off(); 
	if(cbuf_is_empty(&uart0_rx_buf)) return UART_NO_DATA;
	uint8_t data = cbuf_get(&uart0_rx_buf);
	uart0_interrupt_rx_on(); 
	return data;
	//(uart0->UART_LastRxError << 8) + data;
	/*unsigned char tmptail;
	unsigned char data;

	if ( uart0->UART_RxHead == uart0->UART_RxTail ) {
			return UART_NO_DATA;   
	}
	
	tmptail = (uart0->UART_RxTail + 1) & UART_RX_BUFFER_MASK;
	uart0->UART_RxTail = tmptail; 
	
	data = uart0->UART_RxBuf[tmptail];
	
	return (uart0->UART_LastRxError << 8) + data;
*/
}

/*************************************************************************
Function: uart_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none          
**************************************************************************/
void PFDECL(CONFIG_UART0_NAME, putc, unsigned char data)
{
	// the strategy when the buffer is full is to wait for a time and then discard
	// although it could be: discard or wait forever
	int timeout = 100;
	do {
		uart0_interrupt_dre_off();
		int ret = cbuf_put(&uart0_tx_buf, data);
		uart0_interrupt_dre_on();
		if(ret == -1) time_delay(1);
		else break; 
	} while(timeout--);
	
	/*
	unsigned char tmphead;

	tmphead  = (uart0->UART_TxHead + 1) & UART_TX_BUFFER_MASK;
	
	//if(tmphead == uart0->UART_TxTail) return; 
	while ( tmphead == uart0->UART_TxTail ); // wait for free space

	uart0->UART_TxBuf[tmphead] = data;
	uart0->UART_TxHead = tmphead;
	
	UART0_CONTROL    |= _BV(UART0_UDRIE);*/
}


/*************************************************************************
Function: uart_puts()
Purpose:  transmit string to UART
Input:    string to be transmitted
Returns:  none          
**************************************************************************/
void PFDECL(CONFIG_UART0_NAME, puts, const char *s )
{
	while (*s) 
		PFCALL(CONFIG_UART0_NAME, putc, *s++);
}

size_t PFDECL(CONFIG_UART0_NAME, write, const char *s, size_t c)
{
	size_t t = c; 
	while (c--) 
		PFCALL(CONFIG_UART0_NAME, putc, *s++);
	return t; 
}

size_t PFDECL(CONFIG_UART0_NAME, read, const char *s, size_t c)
{
	size_t t = 0; 
	while (t < c) {
		uint16_t d = PFCALL(CONFIG_UART0_NAME, getc);
		if(d == UART_NO_DATA) return t; 
		t++; 
	}
	return t; 
}
uint16_t PFDECL(CONFIG_UART0_NAME, printf, const prog_char *fmt, ...){
	char buf[UART_TX_BUFFER_SIZE * 2]; 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf(buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	PFCALL(CONFIG_UART0_NAME, puts, buf);
	return n; 
}

/*************************************************************************
Function: uart_puts_p()
Purpose:  transmit string from program memory to UART
Input:    program memory string to be transmitted
Returns:  none
**************************************************************************/
void PFDECL(CONFIG_UART0_NAME, puts_p, const char *progmem_s )
{ 
	register char c;
	
	while ( (c = pgm_read_byte(progmem_s++)) ) 
		PFCALL(CONFIG_UART0_NAME, putc, c);

}/* uart_puts_p */

/*
 * these functions are only for ATmegas with two USART
 */
#if defined( ATMEGA_USART1 )

SIGNAL(UART1_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART1 Receive Complete interrupt
Purpose:  called when the UART1 has received a character
**************************************************************************/
{
    unsigned char tmphead;
    unsigned char data;
    unsigned char usr;
    unsigned char lastRxError;
 
 
    /* read UART status register and UART data register */ 
    usr  = UART1_STATUS;
    data = UART1_DATA;
    
    /* */
    lastRxError = (usr & (_BV(FE1)|_BV(DOR1)) );
        
    /* calculate buffer index */ 
    tmphead = ( UART1_RxHead + 1) & UART_RX_BUFFER_MASK;
    
    if ( tmphead == UART1_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    }else{
        /* store new index */
        UART1_RxHead = tmphead;
        /* store received data in buffer */
        UART1_RxBuf[tmphead] = data;
    }
    UART1_LastRxError = lastRxError;   
}


SIGNAL(UART1_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART1 Data Register Empty interrupt
Purpose:  called when the UART1 is ready to transmit the next byte
**************************************************************************/
{
    unsigned char tmptail;

    
    if ( UART1_TxHead != UART1_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART1_TxTail + 1) & UART_TX_BUFFER_MASK;
        UART1_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART1_DATA = UART1_TxBuf[tmptail];  /* start transmission */
    }else{
        /* tx buffer empty, disable UDRE interrupt */
        UART1_CONTROL &= ~_BV(UART1_UDRIE);
    }
}


/*************************************************************************
Function: uart1_init()
Purpose:  initialize UART1 and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void uart1_init(unsigned int baudrate)
{
    UART1_TxHead = 0;
    UART1_TxTail = 0;
    UART1_RxHead = 0;
    UART1_RxTail = 0;
    

    /* Set baud rate */
    if ( baudrate & 0x8000 ) 
    {
    	UART1_STATUS = (1<<U2X1);  //Enable 2x speed 
      baudrate &= ~0x8000;
    }
    UBRR1H = (unsigned char)(baudrate>>8);
    UBRR1L = (unsigned char) baudrate;

    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART1_CONTROL = _BV(RXCIE1)|(1<<RXEN1)|(1<<TXEN1);
    
    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */   
    #ifdef URSEL1
    UCSR1C = (1<<URSEL1)|(3<<UCSZ10);
    #else
    UCSR1C = (3<<UCSZ10);
    #endif 
}/* uart_init */


/*************************************************************************
Function: uart1_getc()
Purpose:  return byte from ringbuffer  
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
unsigned int uart1_getc(void)
{    
    unsigned char tmptail;
    unsigned char data;


    if ( UART1_RxHead == UART1_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }
    
    /* calculate /store buffer index */
    tmptail = (UART1_RxTail + 1) & UART_RX_BUFFER_MASK;
    UART1_RxTail = tmptail; 
    
    /* get data from receive buffer */
    data = UART1_RxBuf[tmptail];
    
    return (UART1_LastRxError << 8) + data;

}/* uart1_getc */


/*************************************************************************
Function: uart1_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none          
**************************************************************************/
void uart1_putc(unsigned char data)
{
    unsigned char tmphead;

    
    tmphead  = (UART1_TxHead + 1) & UART_TX_BUFFER_MASK;
    
    while ( tmphead == UART1_TxTail ){
        ;/* wait for free space in buffer */
    }
    
    UART1_TxBuf[tmphead] = data;
    UART1_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART1_CONTROL    |= _BV(UART1_UDRIE);

}/* uart1_putc */


/*************************************************************************
Function: uart1_puts()
Purpose:  transmit string to UART1
Input:    string to be transmitted
Returns:  none          
**************************************************************************/
void uart1_puts(const char *s )
{
    while (*s) 
      uart1_putc(*s++);

}/* uart1_puts */


/*************************************************************************
Function: uart1_puts_p()
Purpose:  transmit string from program memory to UART1
Input:    program memory string to be transmitted
Returns:  none
**************************************************************************/
void uart1_puts_p(const char *progmem_s )
{
    register char c;
    
    while ( (c = pgm_read_byte(progmem_s++)) ) 
      uart1_putc(c);

}/* uart1_puts_p */


#endif
