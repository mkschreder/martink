UART serial interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/uart.h
* arch/uart.h

Intro
-----

As with many other peripherals, a chip can have several uarts. All uarts should export a serial_interface for generic access. 


Public interface
----------------

Check out definition of struct serial_interface in arch/interface.h 

Implementation guidelines
----------------------

Implementation is as always free to either use buffered interrupt driven uart or simple non buffered uart. The advantage of implementing a buffered uart is that all write operations can return to higher level code much quicker so that application can carry on while uart sends the data in the background. So it is always preferable to at least have a small buffer. 

| Hardware method | Description |
|-----------------|-------------|

// Load value for UBRR into registers
// eg. `uart0_baud(BAUD_PRESCALE_SYNC(57600));`
#define uart0_set_baudrate(ubrr) (UBRR0H = (unsigned char)(ubrr << 8), UBRR0L = (unsigned char)ubrr)

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

#define uart0_init_default(baudrate) ({ \
  uart0_set_baudrate(BAUD_PRESCALE_ASYNC(baudrate)); \
  uart0_mode_async(); \
  uart0_character_size8(); \
  uart0_parity_off(); \
  uart0_stop_1bit(); \
  uart0_receiver_enable(); \
  uart0_transmitter_enable(); \
  uart0_interrupt_rx_on(); \
})

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

#if defined(CONFIG_BUFFERED_UART)
	extern size_t 		uart0_waiting(void); 
	extern void 			uart0_flush(void);
	extern uint16_t 	uart0_getc(void);
	extern uint16_t 	uart0_putc(uint8_t data);
#else
	#define uart0_waiting() ((UCSR0A & _BV(RXC0))?1:0)
	#define uart0_flush() (0)
	#define uart0_getc() ((uart0_waiting())?UDR0:UART_NO_DATA)
	#define uart0_putc(data) uart0_putc_direct(data) 
#endif

Example usage
--------------------

It is up to higher level code to provide printf functionality. However, uart code may export a file descriptor for the uart making it possible to use standard fprintf() functionality pretty easily. If this is done, then global uart file descriptors are exported and named uartX_fd for each available uart. This removes the need to write custom printf functions because we can just use libc. 

```
uart0_init(32400);

// accessing directly
uart0_putc('X');

// using an interface
struct serial_interface serial = uart0_get_serial_interface();
char buffer[] = "Hello World!"; 
serial.putn(&serial, buffer, strlen(buffer));

// using file descriptor
fprintf(uart0_fd, "Hello %s\n", "World!"); 
```
