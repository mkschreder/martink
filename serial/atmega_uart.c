/**
	Fast macro based uart implementation

	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

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
	Github: https://github.com/mkschreder
*/

#include <arch/soc.h>
#include <kernel/cbuf.h>
#include <kernel/mt.h>
#include <kernel/module.h>
#include <string.h>

#include "serial.h"
#include "atmega_uart.h"

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 304
#error "This library requires AVR-GCC 3.4 or later, update to newer AVR-GCC compiler !"
#endif

#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0

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
//<<<<<<< HEAD
#define uart0_set_baudrate_raw(ubrr) (UBRR0H = (unsigned char)(ubrr << 8), UBRR0L = (unsigned char)ubrr)
#define uart0_set_baudrate_async(br) uart0_set_baudrate_raw(BAUD_PRESCALE_ASYNC(br))
/*=======
#define uart0_set_brr(ubrr) (UBRR0H = (unsigned char)(ubrr << 8), UBRR0L = (unsigned char)ubrr)

#define uart0_set_baudrate_async(rate) uart0_set_brr(BAUD_PRESCALE_ASYNC(rate))
// default uart baudrate function
#define uart0_set_baudrate(rate) uart0_set_baudrate_async(rate)
>>>>>>> 8c77070911cdfe42babd9b56e1d39892e745425b*/

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
#define uart0_interrupt_dre_is_on()  (bit_is_set(UCSR0B, UDRIE0))
#define uart0_interrupt_dre_off() (CLRBIT(UCSR0B, UDRIE0))

///// Other /////

// When to transmit data (tx) and receive data in one clock cycle
// Used in synchronous mode only.
// Transmit on falling edge of clock:
#define uart0_clock_polarity_tx_falling() (CLRBIT(UCSR0C, UCPOL0))
// Transmit on raising edge of clock:
#define uart0_clock_polarity_tx_raising() (SETBIT(UCSR0C, UCPOL0))

//TODO this does not compile with -Werror=unused-but-set-variable
#define uart0_flush_receive_buffer(void) {\
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

#define uart0_wait_for_receive_complete()      loop_until_bit_is_set(UCSR0A, RXC0)
#define uart0_wait_for_transmit_complete()     loop_until_bit_is_set(UCSR0A, TXC0)
#define uart0_wait_for_empty_transmit_buffer() loop_until_bit_is_set(UCSR0A, UDRE0)

#define uart0_frame_error() (UCSR0A |= _BV(FE0))
#define uart0_data_overrun() (UCSR0A |= _BV(DOR0))
#define uart0_parity_error() (UCSR0A |= _BV(UPE0))

static inline void uart0_init_default(uint32_t baudrate) { 
  uart0_set_baudrate_raw(BAUD_PRESCALE_ASYNC(baudrate)); 
  uart0_mode_async(); 
  uart0_character_size8(); 
  uart0_parity_off(); 
  uart0_stop_1bit(); 
  uart0_receiver_enable(); 
  uart0_transmitter_enable(); 
  uart0_interrupt_rx_on(); 
}

void uart0_putc_direct(uint8_t ch){
	uart0_wait_for_empty_transmit_buffer();
	UDR0 = ch;
}

/*
static inline uint8_t uart0_getc_direct(void) {
	uart0_wait_for_receive_complete(); 
	return ((uart0_frame_error())?SERIAL_FRAME_ERROR:
	((uart0_data_overrun())?SERIAL_OVERRUN_ERROR:
	((uart0_parity_error())?SERIAL_PARITY_ERROR:
	UDR0))); 
}
*/

struct atmega_uart {
	struct serial_device serial; 
	struct cbuf rx_buf, tx_buf; 
	char rx_buffer[16]; 
	const char *tx_data; 
	uint16_t tx_size, tx_count; 
	mutex_t lock;  // used for synchronizing access to uart
	sem_t rx_ready, tx_ready;
}; 

static struct atmega_uart uart0; 

//DECLARE_STATIC_CBUF(uart0.tx_buf, uint8_t, UART_TX_BUFFER_SIZE);
//DECLARE_STATIC_CBUF(uart0.rx_buf, uint8_t, UART_RX_BUFFER_SIZE);

ISR(USART_RX_vect) { 
	uint8_t err = ( UART0_STATUS & (_BV(FE0)|_BV(DOR0))); 
	uint8_t data = UDR0; 
	if(err) return; 

	if(!cbuf_is_full_isr(&uart0.rx_buf)){ 
		cbuf_put_isr(&uart0.rx_buf, data); 
	}
	// always signal incoming, even if we were not able to place data into the buffer
	mutex_unlock_from_isr(&uart0.rx_ready); 
}

ISR(USART_TX_vect) {
	//mutex_unlock_from_isr(&uart0.tx_ready); 
}

ISR(USART_UDRE_vect) {
	if(uart0.tx_data && uart0.tx_count < uart0.tx_size){
		UDR0 = uart0.tx_data[uart0.tx_count++]; 
	} else {
		uart0_interrupt_dre_off(); 
		mutex_unlock_from_isr(&uart0.tx_ready); 
	}
}

static int atmega_uart_read(struct serial_device *dev, char *data, size_t size){
	struct atmega_uart *self = container_of(dev, struct atmega_uart, serial); 

	// if no characters are available then block until interrupt signals us
	for(;;){
		uart0_interrupt_rx_off(); 
		int empty = cbuf_is_empty(&self->rx_buf); 
		uart0_interrupt_rx_on(); 
		if(!empty) break; 
		// we will only attempt to block if we are empty
#ifdef HAVE_THREADS
		mutex_lock(&self->rx_ready); 
#else
		uart0_wait_for_receive_complete(); 
#endif
	}

	//mutex_lock(&self->lock); 

	uart0_interrupt_rx_off(); 
	int ret = cbuf_getn(&self->rx_buf, data, size); 
	uart0_interrupt_rx_on(); 

	//mutex_unlock(&self->lock); 
	return ret; 
}

static int atmega_uart_write(struct serial_device *dev, const char *data, size_t size){
	struct atmega_uart *self = container_of(dev, struct atmega_uart, serial); 
	mutex_lock(&self->lock); 
	uart0_interrupt_dre_off();
	self->tx_count = 0; 
	self->tx_data = data; 
	self->tx_size = size; 
	uart0_interrupt_dre_on();
#ifdef HAVE_THREADS 
	mutex_lock_timeout(&self->tx_ready, 100); // wait for completion 
#else
	while(uart0_interrupt_dre_is_on()){}; 
#endif
	mutex_unlock(&self->lock); 
	return 0; 
}

static void atmega_uart_init(uint32_t baud){
	struct atmega_uart *self = &uart0; 
	memset(self, 0, sizeof(struct atmega_uart)); 
	cbuf_init(&self->rx_buf, self->rx_buffer, sizeof(self->rx_buffer)); 
	uart0_init_default(baud);
	mutex_init(&self->lock); 
	mutex_init(&self->rx_ready); 
	mutex_init(&self->tx_ready); 
	// lock mutexes here because we will only be unlocking them from isr!
	mutex_lock(&self->rx_ready); 
	mutex_lock(&self->tx_ready);
	uart0_init_default(baud);
}

static void atmega_uart_set_baud(struct serial_device *dev, uint32_t baud){
	struct atmega_uart *self = container_of(dev, struct atmega_uart, serial); 
	mutex_lock(&self->lock); 
	uart0_set_baudrate_raw(BAUD_PRESCALE_ASYNC((uint32_t)baud)); 
	mutex_unlock(&self->lock); 
}

static struct serial_device_ops atmega_uart_ops = {
	.read = atmega_uart_read, 
	.write = atmega_uart_write,
	.set_baud = atmega_uart_set_baud
}; 

static void atmega_uart_probe(void){
	struct atmega_uart *self = &uart0; 
	atmega_uart_init(57600); 

	self->serial.ops = &atmega_uart_ops; 
	
	register_serial_device(&self->serial); 
}

#include "atmega_uart.h"

static void __init _register_driver(void){
	atmega_uart_probe(); 
}

struct serial_device *atmega_uart_get_adapter(void){
	return &uart0.serial; 
}
