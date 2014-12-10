/*
nrf24l01 lib 0x02

copyright (c) Davide Gironi, 2012

References:
  -  This library is based upon nRF24L01 avr lib by Stefan Engelke
     http://www.tinkerer.eu/AVRLib/nRF24L01
  -  and arduino library 2011 by J. Coliz
     http://maniacbug.github.com/RF24

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifndef _NRF24L01_H_
#define _NRF24L01_H_

#ifdef __cplusplus
extern "C" {
#endif

//CE and CSN port definitions
/*#define FX_RADIO*/
/*
#ifdef FX_FLIGHT_CONTROL
#define NRF24L01_DDR DDRD
#define NRF24L01_PORT PORTD
#define NRF24L01_CE PD2
#define NRF24L01_CSN PD3
#elif FX_RADIO
#define NRF24L01_DDR DDRB
#define NRF24L01_PORT PORTB
#define NRF24L01_CE PB0
#define NRF24L01_CSN PB1
#else
#error "FX_FLIGHT_CONTROL or FX_RADIO needs to be defined!"
#endif
*/

#define NRF24L01_DDR *nrf_ddr
#define NRF24L01_PORT *nrf_port
#define NRF24L01_CE nrf_ce_pin
#define NRF24L01_CSN nrf_cs_pin

#define NRF24L01_MAX_CHANNEL 128

//define the spi path
#define NRF24L01_SPIPATH "spi.h" //spi lib path

//CE and CSN functions
#define nrf24l01_CSNhi NRF24L01_PORT |= (1<<NRF24L01_CSN);
#define nrf24l01_CSNlo NRF24L01_PORT &= ~(1<<NRF24L01_CSN);
#define nrf24l01_CEhi NRF24L01_PORT |=  (1<<NRF24L01_CE);
#define nrf24l01_CElo NRF24L01_PORT &= ~(1<<NRF24L01_CE);

//power setup
#define NRF24L01_RF24_PA_MIN 1
#define NRF24L01_RF24_PA_LOW 2
#define NRF24L01_RF24_PA_HIGH 3
#define NRF24L01_RF24_PA_MAX 4
#define NRF24L01_RF24_PA NRF24L01_RF24_PA_MAX

//speed setup
#define NRF24L01_RF24_SPEED_250KBPS 1
#define NRF24L01_RF24_SPEED_1MBPS 2
#define NRF24L01_RF24_SPEED_2MBPS 3
#define NRF24L01_RF24_SPEED NRF24L01_RF24_SPEED_1MBPS

//crc setup
#define NRF24L01_RF24_CRC_DISABLED 1
#define NRF24L01_RF24_CRC_8 2
#define NRF24L01_RF24_CRC_16 3
#define NRF24L01_RF24_CRC NRF24L01_RF24_CRC_16

//transmission channel
#define NRF24L01_CH 54

//payload lenght
#define NRF24L01_PAYLOAD 32

//auto ack enabled
#define NRF24L01_ACK 0

//auto retransmit delay and count
#define NRF24L01_RETR 0xff //1500uS, 15 times

//enable / disable pipe
#define NRF24L01_ENABLEDP0 1 //pipe 0
#define NRF24L01_ENABLEDP1 1 //pipe 1
#define NRF24L01_ENABLEDP2 0 //pipe 2
#define NRF24L01_ENABLEDP3 0 //pipe 3
#define NRF24L01_ENABLEDP4 0 //pipe 4
#define NRF24L01_ENABLEDP5 0 //pipe 5

//address size
#define NRF24L01_ADDRSIZE 5

//pipe address
/*
#define NRF24L01_ADDRP0 {0xE8, 0xE8, 0xF0, 0xF0, 0xE2} //pipe 0, 5 byte address
#define NRF24L01_ADDRP1 {0xC1, 0xC2, 0xC2, 0xC2, 0xC2} //pipe 1, 5 byte address
#define NRF24L01_ADDRP2 {0xC1, 0xC2, 0xC2, 0xC2, 0xC3} //pipe 2, 5 byte address
#define NRF24L01_ADDRP3 {0xC1, 0xC2, 0xC2, 0xC2, 0xC4} //pipe 3, 5 byte address
#define NRF24L01_ADDRP4 {0xC1, 0xC2, 0xC2, 0xC2, 0xC5} //pipe 4, 5 byte address
#define NRF24L01_ADDRP5 {0xC1, 0xC2, 0xC2, 0xC2, 0xC6} //pipe 5, 5 byte address
#define NRF24L01_ADDRTX {0xE8, 0xE8, 0xF0, 0xF0, 0xE2} //tx default address
*/
 //enable print info function
#define NRF24L01_PRINTENABLE 0

extern void nrf24l01_init(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t ce_pin, uint8_t cs_pin);
extern uint8_t nrf24l01_getstatus(void);
extern uint8_t nrf24l01_readready(uint8_t* pipe); 
extern void nrf24l01_read(uint8_t *data);
extern uint8_t nrf24l01_write(uint8_t *data);
extern void nrf24l01_setrxaddr(uint8_t channel, uint8_t *addr);
extern void nrf24l01_settxaddr(uint8_t *addr);
#if NRF24L01_PRINTENABLE == 1
extern void nrf24l01_printinfo(void(*prints)(const char *), void(*printc)(unsigned char data));
void nrf24l01_powerdown(void);
#endif
void nrf24l01_scan(uint8_t iterations, uint8_t result[NRF24L01_MAX_CHANNEL]); 


#ifdef __cplusplus
}
#endif

#endif
