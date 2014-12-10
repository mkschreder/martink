/*
nrf24l01 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <string.h>
#include <stdio.h>

#include <arch/soc.h>

#include "nrf24l01.h"
#include "nrf24l01registers.h"

static volatile uint8_t *nrf_port = 0, *nrf_ddr = 0;
static uint8_t nrf_ce_pin = 0, nrf_cs_pin = 0;

//address variables
/*
static uint8_t nrf24l01_addr0[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP0;
static uint8_t nrf24l01_addr1[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP1;
static uint8_t nrf24l01_addr2[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP2;
static uint8_t nrf24l01_addr3[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP3;
static uint8_t nrf24l01_addr4[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP4;
static uint8_t nrf24l01_addr5[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP5;
static uint8_t nrf24l01_addrtx[NRF24L01_ADDRSIZE] = NRF24L01_ADDRTX;*/

/*
 * read one register
 */
uint8_t nrf24l01_readregister(uint8_t reg) {
	_delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	_delay_us(10); 
	spi_writereadbyte(NRF24L01_CMD_R_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	_delay_us(10); 
	uint8_t result = spi_writereadbyte(NRF24L01_CMD_NOP); //read write register
	_delay_us(10); 
	nrf24l01_CSNhi; //high CSN
	return result;
}

/*
 * read many registers
 */
void nrf24l01_readregisters(uint8_t reg, uint8_t *value, uint8_t len) {
	uint8_t i = 0;
	_delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	_delay_us(10); 
	spi_writereadbyte(NRF24L01_CMD_R_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	_delay_us(10); 
	for(i=0; i<len; i++){
		value[i] = spi_writereadbyte(NRF24L01_CMD_NOP); //read write register
		_delay_us(10); 
	}
	nrf24l01_CSNhi; //high CSN
}

/*
 * write one register
 */
void nrf24l01_writeregister(uint8_t reg, uint8_t value) {
	_delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	_delay_us(10); 
	spi_writereadbyte(NRF24L01_CMD_W_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	_delay_us(10); 
	spi_writereadbyte(value); //write register
	_delay_us(10); 
	nrf24l01_CSNhi; //high CSN
}

/*
 * write many registers
 */
void nrf24l01_writeregisters(uint8_t reg, uint8_t *value, uint8_t len) {
	uint8_t i = 0;
	_delay_us(10); 
	nrf24l01_CSNlo; //low CSN
	_delay_us(10); 
  spi_writereadbyte(NRF24L01_CMD_W_REGISTER | (NRF24L01_CMD_REGISTER_MASK & reg));
	_delay_us(10); 
	for(i=0; i<len; i++){
		 spi_writereadbyte(value[i]); //write register
		_delay_us(10); 
	}
	nrf24l01_CSNhi; //high CSN
}

/*
 * reverse an array, NRF24L01 expects LSB first
 */
void nrf24l01_revaddress(uint8_t *addr, uint8_t *addrrev) {
	//reverse address
	uint8_t i = 0;
	for(i=0; i<NRF24L01_ADDRSIZE; i++)
		memcpy(&addrrev[i], &addr[NRF24L01_ADDRSIZE-1-i], 1);
}

/*
 * set rx address
 */
void nrf24l01_setrxaddr(uint8_t pipe, uint8_t *addr) {
	if(pipe == 0) {
		//memcpy(&nrf24l01_addr0, addr, NRF24L01_ADDRSIZE); //cache address
		uint8_t addrrev[NRF24L01_ADDRSIZE];
		nrf24l01_revaddress(addr, (uint8_t *)addrrev);
    	nrf24l01_writeregisters(NRF24L01_REG_RX_ADDR_P0, addrrev, NRF24L01_ADDRSIZE);
	} else if(pipe == 1) {
		//memcpy(&nrf24l01_addr1, addr, NRF24L01_ADDRSIZE); //cache address
		uint8_t addrrev[NRF24L01_ADDRSIZE];
		nrf24l01_revaddress(addr, (uint8_t *)addrrev);
    	nrf24l01_writeregisters(NRF24L01_REG_RX_ADDR_P1, addrrev, NRF24L01_ADDRSIZE);
	} else if(pipe == 2) {
		//memcpy(&nrf24l01_addr2, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(NRF24L01_REG_RX_ADDR_P2, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	} else if(pipe == 3) {
		//memcpy(&nrf24l01_addr3, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(NRF24L01_REG_RX_ADDR_P3, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	} else if(pipe == 4) {
		//memcpy(&nrf24l01_addr4, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(NRF24L01_REG_RX_ADDR_P4, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	} else if(pipe == 5) {
		//memcpy(&nrf24l01_addr5, addr, NRF24L01_ADDRSIZE); //cache address
		nrf24l01_writeregister(NRF24L01_REG_RX_ADDR_P5, addr[NRF24L01_ADDRSIZE-1]); //write only LSB MSBytes are equal to RX_ADDR_P
	}
}

/*
 * set tx address
 */
void nrf24l01_settxaddr(uint8_t *addr) {
	//memcpy(&nrf24l01_addrtx, addr, NRF24L01_ADDRSIZE); //cache address
	uint8_t addrrev[NRF24L01_ADDRSIZE];
	nrf24l01_revaddress(addr, (uint8_t *)addrrev);
	nrf24l01_writeregisters(NRF24L01_REG_RX_ADDR_P0, addrrev, NRF24L01_ADDRSIZE); //set rx address for ack on pipe 0
	nrf24l01_writeregisters(NRF24L01_REG_TX_ADDR, addrrev, NRF24L01_ADDRSIZE); //set tx address
}

/*
 * flush RX fifo
 */
void nrf24l01_flushRXfifo(void) {
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_FLUSH_RX);
	nrf24l01_CSNhi; //high CSN
}

/*
 * flush RX fifo
 */
void nrf24l01_flushTXfifo(void) {
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_FLUSH_TX);
	nrf24l01_CSNhi; //high CSN
}

/*
 * set chip as RX
 */
void nrf24l01_setRX(void) {
	//nrf24l01_setrxaddr(0, nrf24l01_addr0); //restore pipe 0 address
	nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) | (1<<NRF24L01_REG_PRIM_RX)); //prx mode
	nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) | (1<<NRF24L01_REG_PWR_UP)); //power up
	nrf24l01_writeregister(NRF24L01_REG_STATUS, (1<<NRF24L01_REG_RX_DR) | (1<<NRF24L01_REG_TX_DS) | (1<<NRF24L01_REG_MAX_RT)); //reset status
	nrf24l01_flushRXfifo(); //flush rx fifo
	nrf24l01_flushTXfifo(); //flush tx fifo
	nrf24l01_CEhi; //start listening
	_delay_us(150); //wait for the radio to power up
}

/*
 * set chip as TX
 */
void nrf24l01_setTX(void) {
	nrf24l01_CElo; //stop listening
	nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) & ~(1<<NRF24L01_REG_PRIM_RX)); //ptx mode
	nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) | (1<<NRF24L01_REG_PWR_UP)); //power up
	nrf24l01_writeregister(NRF24L01_REG_STATUS, (1<<NRF24L01_REG_RX_DR) | (1<<NRF24L01_REG_TX_DS) | (1<<NRF24L01_REG_MAX_RT)); //reset status
	nrf24l01_flushTXfifo(); //flush tx fifo
	_delay_us(150); //wait for the radio to power up
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

void nrf24l01_powerdown(void){
	//nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) & ~(1<<NRF24L01_REG_PWR_UP));
	//_delay_ms(10); 
}

/*
 * get status register
 */
uint8_t nrf24l01_getstatus(void) {
	uint8_t status = 0;
	nrf24l01_CSNlo; //low CSN
	status = spi_writereadbyte(NRF24L01_CMD_NOP); //get status, send NOP request
	nrf24l01_CSNhi; //high CSN
	return status;
}

/*
 * check if there is data ready
 */
uint8_t nrf24l01_readready(uint8_t* pipe) {
    uint8_t status = nrf24l01_getstatus();
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
void nrf24l01_read(uint8_t *data) {
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
	nrf24l01_writeregister(NRF24L01_REG_STATUS, (1<<NRF24L01_REG_RX_DR));
	
	//handle ack payload receipt
	if (nrf24l01_getstatus() & (1<<NRF24L01_REG_TX_DS))
		nrf24l01_writeregister(NRF24L01_REG_STATUS, (1<<NRF24L01_REG_TX_DS));

	//power down
	nrf24l01_powerdown(); 
	
	//SREG = sreg; 
}

/*
 * put data
 */
uint8_t nrf24l01_write(uint8_t *data) {
	uint8_t i = 0;
	uint8_t ret = 0;
	
	//uint8_t sreg = SREG; 
	//cli(); 
	
	//set tx mode
	nrf24l01_setTX();

	//write data
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_W_TX_PAYLOAD);
	for (i=0; i<NRF24L01_PAYLOAD; i++)
		spi_writereadbyte(data[i]);
	nrf24l01_CSNhi; //high CSN

	//start transmission
	nrf24l01_CEhi; //high CE
	_delay_us(15);
	nrf24l01_CElo; //low CE
	
	//stop if max_retries reached or send is ok
	uint8_t status = 0; 
	uint32_t timeout = 1500; 
	do {
		_delay_us(15);
		status = nrf24l01_getstatus(); 
		timeout--; if(timeout == 0) break; 
	}
	while( !(status & (1<<NRF24L01_REG_MAX_RT | 1<<NRF24L01_REG_TX_DS)));

	if(status & 1<<NRF24L01_REG_TX_DS)
		ret = 1;
	if(status & 1<<NRF24L01_REG_MAX_RT)
		ret = 2;
		
	//reset PLOS_CNT
	nrf24l01_writeregister(NRF24L01_REG_RF_CH, NRF24L01_CH);

	//power down
	nrf24l01_powerdown();

	//set rx mode
	nrf24l01_setRX();
	
	//SREG = sreg; 
	
	return ret;
}

/*
 * set power level
 */
void nrf24l01_setpalevel(void) {
  uint8_t setup = nrf24l01_readregister(NRF24L01_REG_RF_SETUP);
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

  nrf24l01_writeregister(NRF24L01_REG_RF_SETUP, setup);
}

/*
 * set datarate
 */
void nrf24l01_setdatarate(void) {
  uint8_t setup = nrf24l01_readregister(NRF24L01_REG_RF_SETUP) ;

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

  nrf24l01_writeregister(NRF24L01_REG_RF_SETUP, setup);
}

/*
 * set crc length
 */
void nrf24l01_setcrclength(void) {
  uint8_t config = nrf24l01_readregister(NRF24L01_REG_CONFIG) & ~((1<<NRF24L01_REG_CRCO) | (1<<NRF24L01_REG_EN_CRC));

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

  nrf24l01_writeregister(NRF24L01_REG_CONFIG, config);
}

/*
 * init nrf24l01
 */
void nrf24l01_init(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t ce_pin, uint8_t cs_pin) {
	nrf_ddr = ddr;
	nrf_port = port;
	nrf_ce_pin = ce_pin;
	nrf_cs_pin = cs_pin;
	
	//setup port
	NRF24L01_DDR |= (1<<NRF24L01_CSN); //output
	NRF24L01_DDR |= (1<<NRF24L01_CE); //output

	spi_init(); //init spi

	nrf24l01_CElo; //low CE
	nrf24l01_CSNhi; //high CSN

	_delay_ms(5); //wait for the radio to init

	nrf24l01_setpalevel(); //set power level
	nrf24l01_setdatarate(); //set data rate
	nrf24l01_setcrclength(); //set crc length
	nrf24l01_writeregister(NRF24L01_REG_SETUP_RETR, NRF24L01_RETR); // set retries
	nrf24l01_writeregister(NRF24L01_REG_DYNPD, 0); //disable dynamic payloads
	nrf24l01_writeregister(NRF24L01_REG_RF_CH, NRF24L01_CH); //set RF channel
	//nrf24l01_writeregister(NRF24L01_REG_SETUP_AW, 0x00); 
	
	//payload size
	#if NRF24L01_ENABLEDP0 == 1
		nrf24l01_writeregister(NRF24L01_REG_RX_PW_P0, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP1 == 1
		nrf24l01_writeregister(NRF24L01_REG_RX_PW_P1, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP2 == 1
		nrf24l01_writeregister(NRF24L01_REG_RX_PW_P2, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP3 == 1
		nrf24l01_writeregister(NRF24L01_REG_RX_PW_P3, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP4 == 1
		nrf24l01_writeregister(NRF24L01_REG_RX_PW_P4, NRF24L01_PAYLOAD);
	#endif
	#if NRF24L01_ENABLEDP5 == 1
		nrf24l01_writeregister(NRF24L01_REG_RX_PW_P5, NRF24L01_PAYLOAD);
	#endif

	//enable pipe
	nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, 0);
	#if NRF24L01_ENABLEDP0 == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P0));
	#endif
	#if NRF24L01_ENABLEDP1 == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P1));
	#endif
	#if NRF24L01_ENABLEDP2 == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P2));
	#endif
	#if NRF24L01_ENABLEDP3 == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P3));
	#endif
	#if NRF24L01_ENABLEDP4 == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P4));
	#endif
	#if NRF24L01_ENABLEDP5 == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_RXADDR, nrf24l01_readregister(NRF24L01_REG_EN_RXADDR) | (1<<NRF24L01_REG_ERX_P5));
	#endif

	//auto ack
	#if NRF24L01_ACK == 1
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P0));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P1));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P2));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P3));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P4));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) | (1<<NRF24L01_REG_ENAA_P5));
	#else
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P0));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P1));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P2));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P3));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P4));
		nrf24l01_writeregister(NRF24L01_REG_EN_AA, nrf24l01_readregister(NRF24L01_REG_EN_AA) & ~(1<<NRF24L01_REG_ENAA_P5));
	#endif

	//rx address
	/*nrf24l01_setrxaddr(0, nrf24l01_addr0);
	nrf24l01_setrxaddr(1, nrf24l01_addr1);
	nrf24l01_setrxaddr(2, nrf24l01_addr2);
	nrf24l01_setrxaddr(3, nrf24l01_addr3);
	nrf24l01_setrxaddr(4, nrf24l01_addr4);
	nrf24l01_setrxaddr(5, nrf24l01_addr5);

	//tx address
	nrf24l01_settxaddr(nrf24l01_addrtx);
*/
	//set rx mode
	nrf24l01_setRX();
}


void nrf24l01_scan(uint8_t iterations, uint8_t result[NRF24L01_MAX_CHANNEL]){
	//disable();
	uint8_t channel[NRF24L01_MAX_CHANNEL];
	static const char grey[] PROGMEM = "0123456789"; 
	//char grey[] = " .:-=+*aRW";

	//nrf24l01_powerdown();

	//memset(result, ' ', NRF24L01_MAX_CHANNEL);
	
  for( int j=0 ; j < iterations  ; j++)
  {
    for( int i=0 ; i < NRF24L01_MAX_CHANNEL ; i++)
    {
      // select a new channel
      nrf24l01_writeregister(NRF24L01_REG_RF_CH, i); //set RF channel

      // switch on RX
      nrf24l01_setRX(); 
      
      // wait enough for RX-things to settle
      _delay_us(40);
      
      // this is actually the point where the RPD-flag
      // is set, when CE goes low
      //disable();
      //nrf24l01_powerdown();
      
      // read out RPD flag; set to 1 if 
      // received power > -64dBm
      if( nrf24l01_readregister(NRF24L01_REG_CD) > 0 ) channel[i]++;
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
