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

#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 304
#error "This library requires AVR-GCC 3.4 or later, update to newer AVR-GCC compiler !"
#endif

// used later
#define uart0_clear_errors() (CLRBIT(UCSR0A, FE0), CLRBIT(UCSR0A, DOR0), CLRBIT(UCSR0A, UPE0))

///// Baud Rate /////
/// Values for the UBRRn register depending on USART baud rate
// Asynchronous normal mode (U2Xn = 0)
#define BAUD_PRESCALE_ASYNC(baudr) (((F_CPU / (baudr * 16UL))) - 1)
// Asynchronous double speed mode (U2Xn = 1)
#define BAUD_PRESCALE_ASYNC_DOUBLED(baudr) (((F_CPU / (baudr * 8UL))) - 1)
// Synchronous master mode
#define BAUD_PRESCALE_SYNC(baudr) (((F_CPU / (baudr * 2UL))) - 1)

// Load value for UBRR into registers
// eg. `uart0_baud(BAUD_PRESCALE_SYNC(57600));`
#define uart0_baud(ubrr) (UBRR0H = (unsigned char)(ubrr << 8), UBRR0L = (unsigned char)ubrr)

#define uart0_normal_transmission_speed() (uart0_clear_errors(), CLRBIT(UCSR0A, U2X0))
#define uart0_double_transmission_speed() (uart0_clear_errors(), SETBIT(UCSR0A, U2X0))

///// USART Mode /////
#define uart0_mode_async() (CLRBIT(UCSR0C, UMSEL01), CLRBIT(UCSR0C, UMSEL00))
#define uart0_mode_sync()  (CLRBIT(UCSR0C, UMSEL01), SETBIT(UCSR0C, UMSEL00))
#define uart0_mode_master_spi() (SETBIT(UCSR0C, UMSEL01), SETBIT(UCSR0C, UMSEL00))

///// Frame Format /////
#define uart0_character_size5() (CLRBIT(UCSR0B, UCSZ02), CLRBIT(UCSR0C, UCSZ01), CLRBIT(UCSR0C, UCSZ00))
#define uart0_character_size6() (CLRBIT(UCSR0B, UCSZ02), CLRBIT(UCSR0C, UCSZ01), SETBIT(UCSR0C, UCSZ00))
#define uart0_character_size7() (CLRBIT(UCSR0B, UCSZ02), SETBIT(UCSR0C, UCSZ01), CLRBIT(UCSR0C, UCSZ00))
#define uart0_character_size8() (CLRBIT(UCSR0B, UCSZ02), SETBIT(UCSR0C, UCSZ01), SETBIT(UCSR0C, UCSZ00))
#define uart0_character_size9() (SETBIT(UCSR0B, UCSZ02), SETBIT(UCSR0C, UCSZ01), SETBIT(UCSR0C, UCSZ00))

#define uart0_parity_off()  (CLRBIT(UCSR0C, UPM01), CLRBIT(UCSR0C, UPM00))
#define uart0_parity_even() (SETBIT(UCSR0C, UPM01), CLRBIT(UCSR0C, UPM00))
#define uart0_parity_odd()  (SETBIT(UCSR0C, UPM01), SETBIT(UCSR0C, UPM00))

#define uart0_stop_1bit()  (CLRBIT(UCSR0C, USBS0))
#define uart0_stop_2bits() (SETBIT(UCSR0C, USBS0))

#define uart0_multi_processor_mode_on()  (uart0_clear_errors(), CLRBIT(UCSR0A, MPCM0))
#define uart0_multi_processor_mode_off() (uart0_clear_errors(), SETBIT(UCSR0A, MPCM0))

///// Interrupts /////
#define uart0_interrupt_rx_on()  (SETBIT(UCSR0B, RXCIE0))
#define uart0_interrupt_rx_off() (CLRBIT(UCSR0B, RXCIE0))
#define uart0_interrupt_tx_on()  (SETBIT(UCSR0B, TXCIE0))
#define uart0_interrupt_tx_off() (CLRBIT(UCSR0B, TXCIE0))
#define uart0_interrupt_dre_on()  (SETBIT(UCSR0B, UDRIE0))
#define uart0_interrupt_dre_off() (CLRBIT(UCSR0B, UDRIE0))

///// Other /////

// When to transmit data (tx) and receive data in one clock cycle
// Used in synchronous mode only.
// Transmit on falling edge of clock:
#define uart0_clock_polarity_tx_falling() (CLRBIT(UCSR0C, UCPOL0))
// Transmit on raising edge of clock:
#define uart0_clock_polarity_tx_raising() (SETBIT(UCSR0C, UCPOL0))

//TODO this does not compile with -Werror=unused-but-set-variable
#define uart0_flush_receive_buffer() {\
  unsigned char uart0_flush_receive_buffer_dummy; \
  while (BITSET(UCSR0A, RXC0)) uart0_flush_receive_buffer_dummy = UDR0; \
}

///// Transmitting and receiving /////

#define SETBIT(reg, bit) (reg |= _BV(bit))
#define CLRBIT(reg, bit) (reg &= ~_BV(bit))

#define uart0_receiver_enable()  (SETBIT(UCSR0B, RXEN0))
#define uart0_receiver_disable() (CLRBIT(UCSR0B, RXEN0))
#define uart0_transmitter_enable() (SETBIT(UCSR0B, TXEN0))
#define uart0_transmitter_disable() (CLRBIT(UCSR0B, TXEN0))

#define uart0_receive_complete()  (BITSET(UCSR0A, RXC0))
#define uart0_transmit_complete() (BITSET(UCSR0A, TXC0))
#define uart0_data_register_empty() (BITSET(UCSR0A, UDRE0))

#define uart0_wait_for_receive_complete()      ({loop_until_bit_is_set(UCSR0A, RXC0);})
#define uart0_wait_for_transmit_complete()     ({loop_until_bit_is_set(UCSR0A, TXC0);})
#define uart0_wait_for_empty_transmit_buffer() ({loop_until_bit_is_set(UCSR0A, UDRE0);})

#define uart0_frame_error() (UCSR0A |= _BV(FE0))
#define uart0_data_overrun() (UCSR0A |= _BV(DOR0))
#define uart0_parity_error() (UCSR0A |= _BV(UPE0))

#define uart0_init(baud) PFCALL(uart0, init, baud)

#define uart0_init_default(baudrate) { \
  uart0_baud(BAUD_PRESCALE_ASYNC(baudrate)); \
  uart0_mode_async(); \
  uart0_character_size8(); \
  uart0_parity_off(); \
  uart0_stop_1bit(); \
  uart0_receiver_enable(); \
  uart0_transmitter_enable(); \
  uart0_interrupt_rx_on(); \
}

#define uart0_putc_direct(ch) ({\
	uart0_wait_for_empty_transmit_buffer();\
	UDR0 = ch;\
})

#define uart0_getc_direct() (\
	uart0_wait_for_receive_complete(),\
	((uart0_frame_error())?UART_FRAME_ERROR:\
	((uart0_data_overrun())?UART_OVERRUN_ERROR:\
	((uart0_parity_error())?UART_PARITY_ERROR:\
	UDR0)))\
)

/** Size of the circular receive buffer, must be power of 2 */
#ifndef CONFIG_UART0_TX_BUF_SIZE
#define CONFIG_UART0_TX_BUF_SIZE 32
#endif
/** Size of the circular transmit buffer, must be power of 2 */
#ifndef CONFIG_UART0_RX_BUF_SIZE
#define CONFIG_UART0_RX_BUF_SIZE 32
#endif

#define UART_TX_BUFFER_SIZE CONFIG_UART0_TX_BUF_SIZE
#define UART_RX_BUFFER_SIZE CONFIG_UART0_RX_BUF_SIZE

/* test if the size of the circular buffers fits into SRAM */
#if ( (UART_RX_BUFFER_SIZE+UART_TX_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#warning "size of UART_RX_BUFFER_SIZE + UART_TX_BUFFER_SIZE larger than size of SRAM"
#endif

/* 
** high byte error return code of uart_getc()
*/
#define UART_PARITY_ERROR			0x1000
#define UART_FRAME_ERROR      0x0800              /* Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x0400              /* Overrun condition by UART   */
#define UART_BUFFER_OVERFLOW  0x0200              /* receive ringbuffer overflow */
#define UART_NO_DATA          0x0100              /* no receive data available   */

extern void PFDECL(CONFIG_UART0_NAME, init, unsigned int baudrate);
extern unsigned int PFDECL(CONFIG_UART0_NAME, getc, void);
extern uint16_t PFDECL(CONFIG_UART0_NAME, waiting, void); 
extern void PFDECL(CONFIG_UART0_NAME, putc, unsigned char data);
extern void PFDECL(CONFIG_UART0_NAME, puts, const char *s );
extern size_t PFDECL(CONFIG_UART0_NAME, write, const char *s, size_t c); 
extern size_t PFDECL(CONFIG_UART0_NAME, read, const char *s, size_t c); 
extern void PFDECL(CONFIG_UART0_NAME, puts_p, const char *s );
extern uint16_t PFDECL(CONFIG_UART0_NAME, printf, const prog_char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // UART_H 

