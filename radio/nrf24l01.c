/*
	SPI 2.4Ghz radio driver

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

	Contributions:
	* Davide Gironi - developing original driver
*/

#include <string.h>
#include <stdio.h>

#include <arch/soc.h>

#include "nrf24l01.h"
#include "nrf24l01registers.h"

#define nrf24l01_CSNhi gpio_set(nrf->cs_pin) //NRF24L01_PORT |= (1<<NRF24L01_CSN);
#define nrf24l01_CSNlo gpio_clear(nrf->cs_pin) //NRF24L01_PORT &= ~(1<<NRF24L01_CSN);
#define nrf24l01_CEhi gpio_set(nrf->ce_pin) //NRF24L01_PORT |=  (1<<NRF24L01_CE);
#define nrf24l01_CElo gpio_clear(nrf->ce_pin) // NRF24L01_PORT &= ~(1<<NRF24L01_CE);

#undef spi_init
#define spi_init() {}
#undef spi_writereadbyte
#define spi_writereadbyte(ch) (serial_putc(nrf->spi, ch))

/*
 * read one register
 */
static uint8_t nrf24l01_readregister(struct nrf24l01 *nrf, uint8_t reg) {
	delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	delay_us(10); 
	spi_writereadbyte(NRF24L01_CMD_R_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	delay_us(10); 
	uint8_t result = spi_writereadbyte(NRF24L01_CMD_NOP); //read write register
	delay_us(10); 
	nrf24l01_CSNhi; //high CSN
	return result;
}

/*
 * read many registers
 */
 /*
static void nrf24l01_readregisters(struct nrf24l01 *nrf, uint8_t reg, uint8_t *value, uint8_t len) {
	uint8_t i = 0;
	delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	delay_us(10); 
	spi_writereadbyte(NRF24L01_CMD_R_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	delay_us(10); 
	for(i=0; i<len; i++){
		value[i] = spi_writereadbyte(NRF24L01_CMD_NOP); //read write register
		delay_us(10); 
	}
	nrf24l01_CSNhi; //high CSN
}*/

/*
 * write one register
 */
static void nrf24l01_writeregister(struct nrf24l01 *nrf, uint8_t reg, uint8_t value) {
	delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	delay_us(10); 
	spi_writereadbyte(NRF24L01_CMD_W_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	delay_us(10); 
	spi_writereadbyte(value); //write register
	delay_us(10); 
	nrf24l01_CSNhi; //high CSN
}

/*
 * write many registers
 */
static void nrf24l01_writeregisters(struct nrf24l01 *nrf, uint8_t reg, uint8_t *value, uint8_t len) {
	uint8_t i = 0;
	delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	delay_us(10); 
  spi_writereadbyte(NRF24L01_CMD_W_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	delay_us(10); 
	for(i=0; i<len; i++){
		 spi_writereadbyte(value[i]); //write register
		delay_us(10); 
	}
	nrf24l01_CSNhi; //high CSN
}

/*
 * reverse an array, NRF24L01 expects LSB first
 */
static void nrf24l01_revaddress(uint8_t *addr, uint8_t *addrrev) {
	//reverse address
	uint8_t i = 0;
	for(i=0; i<NRF24L01_ADDRSIZE; i++)
		memcpy(&addrrev[i], &addr[NRF24L01_ADDRSIZE-1-i], 1);
}

/*
 * set rx address
 */
void nrf24l01_setrxaddr(struct nrf24l01 *nrf, uint8_t pipe, uint8_t *addr) {
	if(pipe == 0) {
		//memcpy(&nrf24l01_addr0, addr, NRF24L01_ADDRSIZE); //cache address
		uint8_t addrrev[NRF24L01_ADDRSIZE];
		nrf24l01_revaddress(addr, (uint8_t *)addrrev);
    	nrf24l01_writeregisters(nrf, NRF24L01_REG_RX_ADDR_P0, addrrev, NRF24L01_ADDRSIZE);
	} else if(pipe == 1) {
		//memcpy(&nrf24l01_addr1, addr, NRF24L01_ADDRSIZE); //cache address
		uint8_t addrrev[NRF24L01_ADDRSIZE];
		nrf24l01_revaddress(addr, (uint8_t *)addrrev);
    	nrf24l01_writeregisters(nrf, NRF24L01_REG_RX_ADDR_P1, addrrev, NRF24L01_ADDRSIZE);
	} else if(pipe == 2) {
		//memcpy(&nrf24l01_addr2, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_ADDR_P2, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	} else if(pipe == 3) {
		//memcpy(&nrf24l01_addr3, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_ADDR_P3, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	} else if(pipe == 4) {
		//memcpy(&nrf24l01_addr4, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_ADDR_P4, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	} else if(pipe == 5) {
		//memcpy(&nrf24l01_addr5, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_ADDR_P5, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	}
}

/*
 * set tx address
 */
void nrf24l01_settxaddr(struct nrf24l01 *nrf, uint8_t *addr) {
	//memcpy(&nrf24l01_addrtx, addr, NRF24L01_ADDRSIZE); //cache address
	uint8_t addrrev[NRF24L01_ADDRSIZE];
	nrf24l01_revaddress(addr, (uint8_t *)addrrev);
	nrf24l01_writeregisters(nrf, NRF24L01_REG_RX_ADDR_P0, addrrev, NRF24L01_ADDRSIZE); //set rx address for ack on pipe 0
	nrf24l01_writeregisters(nrf, NRF24L01_REG_TX_ADDR, addrrev, NRF24L01_ADDRSIZE); //set tx address
}

/*
 * flush RX fifo
 */
static void nrf24l01_flushRXfifo(struct nrf24l01 *nrf) {
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_FLUSH_RX);
	nrf24l01_CSNhi; //high CSN
}

/*
 * flush RX fifo
 */
static void nrf24l01_flushTXfifo(struct nrf24l01 *nrf) {
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_FLUSH_TX);
	nrf24l01_CSNhi; //high CSN
}

/*
 * set chip as RX
 */
static void nrf24l01_setRX(struct nrf24l01 *nrf) {
	//nrf24l01_setrxaddr(0, nrf24l01_addr0); //restore pipe 0 address
	nrf24l01_writeregister(nrf, NRF24L01_REG_CONFIG, nrf24l01_readregister(nrf, NRF24L01_REG_CONFIG) | (1<<NRF24L01_REG_PRIM_RX)); //prx mode
	nrf24l01_writeregister(nrf, NRF24L01_REG_CONFIG, nrf24l01_readregister(nrf, NRF24L01_REG_CONFIG) | (1<<NRF24L01_REG_PWR_UP)); //power up
	nrf24l01_writeregister(nrf, NRF24L01_REG_STATUS, (1<<NRF24L01_REG_RX_DR) | (1<<NRF24L01_REG_TX_DS) | (1<<NRF24L01_REG_MAX_RT)); //reset status
	nrf24l01_flushRXfifo(nrf); //flush rx fifo
	nrf24l01_flushTXfifo(nrf); //flush tx fifo
	nrf24l01_CEhi; //start listening
	delay_us(150); //wait for the radio to power up
}

/*
 * set chip as TX
 */
static void nrf24l01_setTX(struct nrf24l01 *nrf) {
	nrf24l01_CElo; //stop listening
	nrf24l01_writeregister(nrf, NRF24L01_REG_CONFIG, nrf24l01_readregister(nrf, NRF24L01_REG_CONFIG) & ~(1<<NRF24L01_REG_PRIM_RX)); //ptx mode
	nrf24l01_writeregister(nrf, NRF24L01_REG_CONFIG, nrf24l01_readregister(nrf, NRF24L01_REG_CONFIG) | (1<<NRF24L01_REG_PWR_UP)); //power up
	nrf24l01_writeregister(nrf, NRF24L01_REG_STATUS, (1<<NRF24L01_REG_RX_DR) | (1<<NRF24L01_REG_TX_DS) | (1<<NRF24L01_REG_MAX_RT)); //reset status
	nrf24l01_flushTXfifo(nrf); //flush tx fifo
	delay_us(150); //wait for the radio to power up
}

/*

extern void uart_printf(const char *fmt, ...); 
void nrf24l01_printinfo(void(*prints)(const char *), void(*printc)(unsigned char data)) {
	prints("info\n");
	uint8_t r = nrf24l01_getstatus(); 
	uart_printf("STATUS: TX_FULL: %d, RX_P_NO: %d, MAX_RT: %d, TX_DS: %d, RX_DR: %d\n",
		r & 0x01, (r >> 1) & 0x07, (r >> 4) & 1, (r >> 6) & 1, (r >> 7) & 1);
	r = nrf24l01_readregister(NRF24L01_REG_CONFIG);
	uart_printf("CONFIG: PRIM_RX: %d, PWR_UP: %d, CRCO: %d, EN_CRC: %d, RT_INT: %d, TX_INT: %d, RX_INT: %d\n", (r) & 1, (r >> 1) & 1, (r >> 2) & 1, (r >> 3) & 1, (r >> 4) & 1,
		 (r >> 5) & 1, (r >> 6) & 1, (r >> 7) & 1);
	r = nrf24l01_readregister(NRF24L01_REG_EN_AA);
	uart_printf("EN_AA: ");
	for(int c = 0; c < 6; c++)
		uart_printf("PIPE%d: %d ", c, (r >> c) & 1);
	uart_printf("\n");
	r = nrf24l01_readregister(NRF24L01_REG_EN_RXADDR);
	uart_printf("EN_RX_ADDR: ");
	for(int c = 0; c < 6; c++)
		uart_printf("PIPE%d: %d ", c, (r >> c) & 1);
	uart_printf("\n");
	r = nrf24l01_readregister(NRF24L01_REG_SETUP_AW);
	uart_printf("SETUP_AW: %d\n", r & 3);
	r = nrf24l01_readregister(NRF24L01_REG_SETUP_RETR);
	uart_printf("SETUP_RETR: RT_COUNT: %d, RT_DELAY: %d\n", r & 0x7, (r >> 4) & 0x7);
	r = nrf24l01_readregister(NRF24L01_REG_RF_CH);
	uart_printf("RF_CH: %d\n", r);
	r = nrf24l01_readregister(NRF24L01_REG_RF_SETUP);
	uart_printf("RF_SETUP: LNA: %d, RF_PWR: %d, RF_DR: %d, PLL: %d\n", r & 1, (r >> 1) & 3, (r >> 3) & 1, (r >> 4) & 1);
	r = nrf24l01_readregister(NRF24L01_REG_OBSERVE_TX);
	uart_printf("OBSERVE_TX: RT_COUNT: %d, LOST_COUNT: %d\n", r & 0xf, (r >> 4) & 0xf);
	
}
*/

void nrf24l01_powerdown(struct nrf24l01 *nrf){
	(void)(nrf); 
	//nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) & ~(1<<NRF24L01_REG_PWR_UP));
	//_delay_ms(10); 
}

/*
 * get status register
 */
uint8_t nrf24l01_getstatus(struct nrf24l01 *nrf) {
	uint8_t status = 0;
	nrf24l01_CSNlo; //low CSN
	status = spi_writereadbyte(NRF24L01_CMD_NOP); //get status, send NOP request
	nrf24l01_CSNhi; //high CSN
	return status;
}

/*
 * check if there is data ready
 */
uint8_t nrf24l01_readready(struct nrf24l01 *nrf, uint8_t* pipe) {
    uint8_t status = nrf24l01_getstatus(nrf);
    uint8_t ret = status & (1<<NRF24L01_REG_RX_DR);
    if(ret) {
			//get the pipe number
			if(pipe)
				*pipe = (status >> NRF24L01_REG_RX_P_NO) & 0b111;
    }
    return ret;
}

/*
 * get data
 */
void nrf24l01_read(struct nrf24l01 *nrf, uint8_t *data) {
	uint8_t i = 0;
	//read rx register
	
	//uint8_t sreg = SREG; 
	//cli(); 
	
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_R_RX_PAYLOAD);
	for(i=0; i<NRF24L01_PAYLOAD; i++)
		data[i] = spi_writereadbyte(NRF24L01_CMD_NOP);
	nrf24l01_CSNhi; //high CSN
	//reset register
	nrf24l01_writeregister(nrf, NRF24L01_REG_STATUS, (1<<NRF24L01_REG_RX_DR));
	
	//handle ack payload receipt
	if (nrf24l01_getstatus(nrf) & (1<<NRF24L01_REG_TX_DS))
		nrf24l01_writeregister(nrf, NRF24L01_REG_STATUS, (1<<NRF24L01_REG_TX_DS));

	//power down
	nrf24l01_powerdown(nrf); 
	
	//SREG = sreg; 
}

/*
 * put data
 */
uint8_t nrf24l01_write(struct nrf24l01 *nrf, uint8_t *data) {
	uint8_t i = 0;
	uint8_t ret = 0;
	
	//uint8_t sreg = SREG; 
	//cli(); 
	
	//set tx mode
	nrf24l01_setTX(nrf);

	//write data
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_W_TX_PAYLOAD);
	for (i=0; i<NRF24L01_PAYLOAD; i++)
		spi_writereadbyte(data[i]);
	nrf24l01_CSNhi; //high CSN

	//start transmission
	nrf24l01_CEhi; //high CE
	delay_us(15);
	nrf24l01_CElo; //low CE
	
	//stop if max_retries reached or send is ok
	uint8_t status = 0; 
	uint32_t timeout = 1500; 
	do {
		delay_us(15);
		status = nrf24l01_getstatus(nrf); 
		timeout--; if(timeout == 0) break; 
	}
	while( !(status & (1<<NRF24L01_REG_MAX_RT | 1<<NRF24L01_REG_TX_DS)));

	if(status & 1<<NRF24L01_REG_TX_DS)
		ret = 1;
	if(status & 1<<NRF24L01_REG_MAX_RT)
		ret = 2;
		
	//reset PLOS_CNT
	nrf24l01_writeregister(nrf, NRF24L01_REG_RF_CH, NRF24L01_CH);

	//power down
	nrf24l01_powerdown(nrf);

	//set rx mode
	nrf24l01_setRX(nrf);
	
	//SREG = sreg; 
	
	return ret;
}

/*
 * set power level
 */
static void nrf24l01_setpalevel(struct nrf24l01 *nrf) {
  uint8_t setup = nrf24l01_readregister(nrf, NRF24L01_REG_RF_SETUP);
  setup &= ~((1<<NRF24L01_REG_RF_PWR_LOW) | (1<<NRF24L01_REG_RF_PWR_HIGH));

  if (NRF24L01_RF24_PA == NRF24L01_RF24_PA_MAX) {
	  setup |= (1<<NRF24L01_REG_RF_PWR_LOW) | (1<<NRF24L01_REG_RF_PWR_HIGH);
  } else if (NRF24L01_RF24_PA == NRF24L01_RF24_PA_HIGH) {
	  setup |= (1<<NRF24L01_REG_RF_PWR_HIGH) ;
  } else if (NRF24L01_RF24_PA == NRF24L01_RF24_PA_LOW) {
	  setup |= (1<<NRF24L01_REG_RF_PWR_LOW);
  } else if (NRF24L01_RF24_PA == NRF24L01_RF24_PA_MIN) {
  } else {
	  //default is max power
	  setup |= (1<<NRF24L01_REG_RF_PWR_LOW) | (1<<NRF24L01_REG_RF_PWR_HIGH);
  }

  nrf24l01_writeregister(nrf, NRF24L01_REG_RF_SETUP, setup);
}

/*
 * set datarate
 */
static void nrf24l01_setdatarate(struct nrf24l01 *nrf) {
  uint8_t setup = nrf24l01_readregister(nrf, NRF24L01_REG_RF_SETUP) ;

  setup &= ~((1<<NRF24L01_REG_RF_DR_LOW) | (1<<NRF24L01_REG_RF_DR_HIGH));
  if(NRF24L01_RF24_SPEED == NRF24L01_RF24_SPEED_250KBPS) {
    setup |= (1<<NRF24L01_REG_RF_DR_LOW);
  } else {
    if (NRF24L01_RF24_SPEED == NRF24L01_RF24_SPEED_2MBPS) {
    	setup |= (1<<NRF24L01_REG_RF_DR_HIGH);
    } else {
    	//default is 1Mbps
    }
  }

  nrf24l01_writeregister(nrf, NRF24L01_REG_RF_SETUP, setup);
}

/*
 * set crc length
 */
static void nrf24l01_setcrclength(struct nrf24l01 *nrf) {
  uint8_t config = nrf24l01_readregister(nrf, NRF24L01_REG_CONFIG) & ~((1<<NRF24L01_REG_CRCO) | (1<<NRF24L01_REG_EN_CRC));

  if (NRF24L01_RF24_CRC == NRF24L01_RF24_CRC_DISABLED) {
	  //nothing
  } else if (NRF24L01_RF24_CRC == NRF24L01_RF24_CRC_8) {
	  config |= (1<<NRF24L01_REG_EN_CRC);
  } else if (NRF24L01_RF24_CRC == NRF24L01_RF24_CRC_16) {
	  config |= (1<<NRF24L01_REG_EN_CRC);
	  config |= (1<<NRF24L01_REG_CRCO);
  } else {
	  //default is disabled
  }

  nrf24l01_writeregister(nrf, NRF24L01_REG_CONFIG, config);
}

/*
 * init nrf24l01
 */
void nrf24l01_init(struct nrf24l01 *nrf, serial_dev_t spi, gpio_pin_t cs, gpio_pin_t ce) {
	nrf->spi = spi; 
	nrf->cs_pin = cs; 
	nrf->ce_pin = ce; 
	
	gpio_configure(nrf->cs_pin, GP_OUTPUT); 
	gpio_configure(nrf->ce_pin, GP_OUTPUT); 
	
	//spi_init(); //init spi

	nrf24l01_CElo; //low CE
	nrf24l01_CSNhi; //high CSN

	delay_us(5000L); //wait for the radio to init

	nrf24l01_setpalevel(nrf); //set power level
	nrf24l01_setdatarate(nrf); //set data rate
	nrf24l01_setcrclength(nrf); //set crc length
	nrf24l01_writeregister(nrf, NRF24L01_REG_SETUP_RETR, NRF24L01_RETR); // set retries
	nrf24l01_writeregister(nrf, NRF24L01_REG_DYNPD, 0); //disable dynamic payloads
	nrf24l01_writeregister(nrf, NRF24L01_REG_RF_CH, NRF24L01_CH); //set RF channel
	//nrf24l01_writeregister(NRF24L01_REG_SETUP_AW, 0x00); 
	
	//payload size
	#if NRF24L01_ENABLEDP0 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_PW_P0, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP1 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_PW_P1, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP2 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_PW_P2, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP3 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_PW_P3, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP4 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_PW_P4, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP5 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_RX_PW_P5, NRF24L01_PAYLOAD);
	#endif

	//enable pipe
	nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, 0);
	#if NRF24L01_ENABLEDP0 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(nrf, NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P0));
	#endif
	#if NRF24L01_ENABLEDP1 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(nrf, NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P1));
	#endif
	#if NRF24L01_ENABLEDP2 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(nrf, NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P2));
	#endif
	#if NRF24L01_ENABLEDP3 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(nrf, NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P3));
	#endif
	#if NRF24L01_ENABLEDP4 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(nrf, NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P4));
	#endif
	#if NRF24L01_ENABLEDP5 == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(nrf, NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P5));
	#endif

	//auto ack
	#if NRF24L01_ACK == 1
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P0));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P1));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P2));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P3));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P4));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P5));
	#else
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P0));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P1));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P2));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P3));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P4));
		nrf24l01_writeregister(nrf, NRF24L01_REG_EN_AA, nrf24l01_readregister(nrf, NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P5));
	#endif

	//set rx mode
	nrf24l01_setRX(nrf);
}


void nrf24l01_scan(struct nrf24l01 *nrf, uint8_t iterations, uint8_t result[NRF24L01_MAX_CHANNEL]){
	//disable();
	uint8_t channel[NRF24L01_MAX_CHANNEL];
	static const char grey[] PROGMEM = " 123456789"; 
	//static const char grey[] PROGMEM = " .:-=+*aRW"; 
	//char grey[] = " .:-=+*aRW";

	//nrf24l01_powerdown();
	memset(channel, 0, sizeof(channel)); 
	memset(result, ' ', NRF24L01_MAX_CHANNEL);
	
  for( int j=0 ; j < iterations  ; j++)
  {
    for( int i=0 ; i < NRF24L01_MAX_CHANNEL ; i++)
    {
      // select a new channel
      nrf24l01_writeregister(nrf, NRF24L01_REG_RF_CH, i); //set RF channel

      // switch on RX
      nrf24l01_setRX(nrf); 
      
      // wait enough for RX-things to settle
      delay_us(40);
      
      // this is actually the point where the RPD-flag
      // is set, when CE goes low
      //disable();
      //nrf24l01_powerdown();
      
      // read out RPD flag; set to 1 if 
      // received power > -64dBm
      if( nrf24l01_readregister(nrf, NRF24L01_REG_CD) > 0 ) channel[i]++;
    }
  }
  uint32_t norm = 0;
  
  // find the maximal count in channel array
  for( int i=0 ; i < NRF24L01_MAX_CHANNEL ; i++)
    if( channel[i] > norm ) norm = channel[i];
    
  // compute normalized values
  for( int i=0 ; i < NRF24L01_MAX_CHANNEL ; i++){
    int pos;
    
    // calculate grey value position
    if( norm!=0 ) pos = (channel[i]*10)/norm;
    else          pos = 0;
    
    // boost low values
    if( pos==0 && channel[i]>0 ) pos++;
    
    // clamp large values
    if( pos>9 ) pos = 9;
   
    // print it out
    result[i] = pgm_read_byte(&grey[pos]);
    channel[i] = 0;
  }
}
