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

#include <inttypes.h>
#include <stdarg.h>
#include <string.h>

#include <arch/soc.h>

#define BUFFER_LENGTH 32 
#define XMIT_TIMEOUT 1000

typedef enum {
	UNINITIALIZED,
	MASTER_IDLE,
	MASTER_SEND,
	MASTER_RECV,
	SLAVE_IDLE,
	SLAVE_RECV,
	SLAVE_SEND
} twi_state_t ;

struct twi {
	uint8_t txaddr; 
	uint8_t buflen; 
	// RX Buffer
	uint8_t rxBuffer[BUFFER_LENGTH];
	uint8_t rxBufferIndex;
	uint8_t rxBufferLength;

	// TX Buffer
	uint8_t txAddress;
	uint8_t txBuffer[BUFFER_LENGTH];
	uint8_t txBufferLength;

	// Service buffer
	uint8_t srvBuffer[BUFFER_LENGTH];
	uint8_t srvBufferIndex;
	uint8_t srvBufferLength;

	// TWI instance
	Twi *twi;

	twi_state_t status;

	// TWI clock frequency
	uint32_t twiClock;

}; 

static struct twi twi0;

static inline uint8_t TWI_FailedAcknowledge(Twi *pTwi) {
	return pTwi->TWI_SR & TWI_SR_NACK;
}

static inline uint8_t TWI_WaitTransferComplete(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
			return 0;

		if (--_timeout == 0)
			return 0;
	}
	return 1;
}

static inline uint8_t TWI_WaitByteSent(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_TXRDY) != TWI_SR_TXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
			return 0;

		if (--_timeout == 0)
			return 0;
	}

	return 1;
}

static inline uint8_t TWI_WaitByteReceived(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_RXRDY) != TWI_SR_RXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
			return 0;

		if (--_timeout == 0)
			return 0;
	}

	return 1;
}

static inline uint8_t TWI_STATUS_SVREAD(uint32_t status) {
	return (status & TWI_SR_SVREAD) == TWI_SR_SVREAD;
}

static inline uint8_t TWI_STATUS_SVACC(uint32_t status) {
	return (status & TWI_SR_SVACC) == TWI_SR_SVACC;
}

static inline uint8_t TWI_STATUS_GACC(uint32_t status) {
	return (status & TWI_SR_GACC) == TWI_SR_GACC;
}

static inline uint8_t TWI_STATUS_EOSACC(uint32_t status) {
	return (status & TWI_SR_EOSACC) == TWI_SR_EOSACC;
}

static inline uint8_t TWI_STATUS_NACK(uint32_t status) {
	return (status & TWI_SR_NACK) == TWI_SR_NACK;
}


void __twi0_init__(){
	pmc_enable_periph_clk(ID_PIOA);
  pmc_enable_periph_clk(ID_TWI0);

	PIO_Configure(PIOA, PIO_PERIPH_A, 
		(PIO_PA18A_TWCK0|PIO_PA17A_TWD0), PIO_DEFAULT); 
	
	NVIC_DisableIRQ(TWI0_IRQn);
	NVIC_ClearPendingIRQ(TWI0_IRQn);
	NVIC_SetPriority(TWI0_IRQn, 0);
	NVIC_EnableIRQ(TWI0_IRQn);
	//TWI0->TWI_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;
	//TWI_ConfigureMaster(TWI0, 100000L, SystemCoreClock);
}

uint8_t __twi0_start__(uint8_t addr){
	
	// Disable PDC channel
	TWI0->TWI_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

	TWI_ConfigureMaster(TWI0, 100000L, SystemCoreClock);
	twi0.status = MASTER_IDLE;
	
	twi0.status = MASTER_SEND;

	// save address of target and empty buffer
	twi0.txaddr = addr;
	twi0.buflen = 0;
	
	/*
	TWI_SendSTOPCondition(TWI0);
	if(addr & I2C_READ)
		TWI_StartRead(TWI0, addr, 0, 0);
	else
		TWI_StartWrite(TWI0, addr, 0, 0, 0);
	*/
	return 1;
}


uint8_t __twi0_end__(uint8_t sendStop) {
	uint8_t error = 0;
	// transmit buffer (blocking)
	TWI_StartWrite(TWI0, twi0.txaddr, 0, 0, twi0.txBuffer[0]);
	if (!TWI_WaitByteSent(TWI0, XMIT_TIMEOUT))
		error = 2;	// error, got NACK on address transmit
	
	if (error == 0) {
		uint16_t sent = 1;
		while (sent < twi0.txBufferLength) {
			TWI_WriteByte(TWI0, twi0.txBuffer[sent++]);
			if (!TWI_WaitByteSent(TWI0, XMIT_TIMEOUT))
				error = 3;	// error, got NACK during data transmmit
		}
	}
	
	if (error == 0) {
		TWI_Stop(TWI0);
		if (!TWI_WaitTransferComplete(TWI0, XMIT_TIMEOUT))
			error = 4;	// error, finishing up
	}

	twi0.txBufferLength = 0;		// empty buffer
	twi0.status = MASTER_IDLE;
	return error;
}

void __twi0_stop__(void){
	__twi0_end__(1); 
}

uint8_t PFDECL(CONFIG_TWI0_NAME, write, uint8_t data) {
	if (twi0.status == MASTER_SEND) {
		if (twi0.txBufferLength >= BUFFER_LENGTH)
			return 0;
		twi0.txBuffer[twi0.txBufferLength++] = data;
		return 1;
	} else {
		if (twi0.srvBufferLength >= BUFFER_LENGTH)
			return 0;
		twi0.srvBuffer[twi0.srvBufferLength++] = data;
		return 1;
	}
}

size_t __twi0_write_buffer__(const uint8_t *data, size_t quantity) {
	if (twi0.status == MASTER_SEND) {
		for (size_t i = 0; i < quantity; ++i) {
			if (twi0.txBufferLength >= BUFFER_LENGTH)
				return i;
			twi0.txBuffer[twi0.txBufferLength++] = data[i];
		}
	} else {
		for (size_t i = 0; i < quantity; ++i) {
			if (twi0.srvBufferLength >= BUFFER_LENGTH)
				return i;
			twi0.srvBuffer[twi0.srvBufferLength++] = data[i];
		}
	}
	return quantity;
}

int __twi0_available__(void) {
	return twi0.rxBufferLength - twi0.rxBufferIndex;
}

int __twi0_read__(void) {
	if (twi0.rxBufferIndex < twi0.rxBufferLength)
		return twi0.rxBuffer[twi0.rxBufferIndex++];
	return -1;
}

int __twi0_peek__(void) {
	if (twi0.rxBufferIndex < twi0.rxBufferLength)
		return twi0.rxBuffer[twi0.rxBufferIndex];
	return -1;
}

uint8_t __twi0_slave_start(uint8_t myaddr){
	
	// Disable PDC channel
	TWI0->TWI_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

	TWI_ConfigureSlave(TWI0, myaddr);
	twi0.status = SLAVE_IDLE;
	TWI_EnableIt(TWI0, TWI_IER_SVACC);
	
	return 1; 
}

uint8_t __twi0_start_wait__(uint8_t addr){
	__twi0_end__(1);
	/*
	if(addr & I2C_READ)
		TWI_StartRead(TWI0, addr, 0, 0);
	else
		TWI_StartWrite(TWI0, addr, 0, 0, 0);
	*/
	return 1;
}
/*
uint8_t __twi0_readNak__(void){
	__twi0_end__(0); 
	while (!TWI_ByteReceived(TWI0)) ;
	uint8_t ret = TWI_ReadByte(TWI0);
	return ret; 
}

uint8_t __twi0_readAck__(void){
	return __twi0_readNak__(); 
}

void __twi0_stop__(void){
	TWI_SendSTOPCondition(TWI0);
}

uint8_t __twi0_write__(uint8_t byte){
	TWI_WriteByte(TWI0, byte);
	while(!TWI_ByteSent(TWI0));
	return 1; 
	//while (!TWI_TransferComplete(TWI0));
}
*/

void TWI0_Handler(void){
	// Retrieve interrupt status
	uint32_t sr = TWI_GetStatus(TWI0);
	struct twi *twi = &twi0; 
	
	if (twi->status == SLAVE_IDLE && TWI_STATUS_SVACC(sr)) {
		TWI_DisableIt(TWI0, TWI_IDR_SVACC);
		TWI_EnableIt(TWI0, TWI_IER_RXRDY | TWI_IER_GACC | TWI_IER_NACK
				| TWI_IER_EOSACC | TWI_IER_SCL_WS | TWI_IER_TXCOMP);

		twi->srvBufferLength = 0;
		twi->srvBufferIndex = 0;

		// Detect if we should go into RECV or SEND status
		// SVREAD==1 means *master* reading -> SLAVE_SEND
		if (!TWI_STATUS_SVREAD(sr)) {
			twi->status = SLAVE_RECV;
		} else {
			twi->status = SLAVE_SEND;

			// create a default 1-byte response
			__twi0_write__((uint8_t) 0);
		}
	}

	if (twi->status != SLAVE_IDLE) {
		if (TWI_STATUS_TXCOMP(sr) && TWI_STATUS_EOSACC(sr)) {
			if (twi->status == SLAVE_RECV) {
				// Copy data into rxBuffer
				// (allows to receive another packet while the
				// user program reads actual data)
				for (uint8_t i = 0; i < twi->srvBufferLength; ++i)
					twi->rxBuffer[i] = twi->srvBuffer[i];
				twi->rxBufferIndex = 0;
				twi->rxBufferLength = twi->srvBufferLength;
			}

			// Transfer completed
			TWI_EnableIt(TWI0, TWI_SR_SVACC);
			TWI_DisableIt(TWI0, TWI_IDR_RXRDY | TWI_IDR_GACC | TWI_IDR_NACK
					| TWI_IDR_EOSACC | TWI_IDR_SCL_WS | TWI_IER_TXCOMP);
			twi->status = SLAVE_IDLE;
		}
	}

	if (twi->status == SLAVE_RECV) {
		if (TWI_STATUS_RXRDY(sr)) {
			if (twi->srvBufferLength < BUFFER_LENGTH)
				twi->srvBuffer[twi->srvBufferLength++] = TWI_ReadByte(TWI0);
		}
	}

	if (twi->status == SLAVE_SEND) {
		if (TWI_STATUS_TXRDY(sr) && !TWI_STATUS_NACK(sr)) {
			uint8_t c = 'x';
			if (twi->srvBufferIndex < twi->srvBufferLength)
				c = twi->srvBuffer[twi->srvBufferIndex++];
			TWI_WriteByte(TWI0, c);
		}
	}
}

/*
void __twi0_set_clock__(uint32_t frequency) {
	TWI_SetClock(TWI0, frequency, VARIANT_MCK);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
	if (quantity > BUFFER_LENGTH)
		quantity = BUFFER_LENGTH;

	// perform blocking read into buffer
	int readed = 0;
	TWI_StartRead(TWI0, address, 0, 0);
	do {
		// Stop condition must be set during the reception of last byte
		if (readed + 1 == quantity)
			TWI_SendSTOPCondition( TWI0);

		TWI_WaitByteReceived(TWI0, RECV_TIMEOUT);
		rxBuffer[readed++] = TWI_ReadByte(TWI0);
	} while (readed < quantity);
	TWI_WaitTransferComplete(TWI0, RECV_TIMEOUT);

	// set rx buffer iterator vars
	rxBufferIndex = 0;
	rxBufferLength = readed;

	return readed;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
	return requestFrom((uint8_t) address, (uint8_t) quantity, (uint8_t) true);
}

uint8_t TwoWire::requestFrom(int address, int quantity) {
	return requestFrom((uint8_t) address, (uint8_t) quantity, (uint8_t) true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop) {
	return requestFrom((uint8_t) address, (uint8_t) quantity, (uint8_t) sendStop);
}
*/

/*
#if WIRE_INTERFACES_COUNT > 0
static void Wire_Init(void) {
	pmc_enable_periph_clk(WIRE_INTERFACE_ID);
	PIO_Configure(
			g_APinDescription[PIN_WIRE_SDA].pPort,
			g_APinDescription[PIN_WIRE_SDA].ulPinType,
			g_APinDescription[PIN_WIRE_SDA].ulPin,
			g_APinDescription[PIN_WIRE_SDA].ulPinConfiguration);
	PIO_Configure(
			g_APinDescription[PIN_WIRE_SCL].pPort,
			g_APinDescription[PIN_WIRE_SCL].ulPinType,
			g_APinDescription[PIN_WIRE_SCL].ulPin,
			g_APinDescription[PIN_WIRE_SCL].ulPinConfiguration);

	NVIC_DisableIRQ(WIRE_ISR_ID);
	NVIC_ClearPendingIRQ(WIRE_ISR_ID);
	NVIC_SetPriority(WIRE_ISR_ID, 0);
	NVIC_EnableIRQ(WIRE_ISR_ID);
}

TwoWire Wire = TwoWire(WIRE_INTERFACE, Wire_Init);

void WIRE_ISR_HANDLER(void) {
	Wire.onService();
}
#endif

#if WIRE_INTERFACES_COUNT > 1
static void Wire1_Init(void) {
	pmc_enable_periph_clk(WIRE1_INTERFACE_ID);
	PIO_Configure(
			g_APinDescription[PIN_WIRE1_SDA].pPort,
			g_APinDescription[PIN_WIRE1_SDA].ulPinType,
			g_APinDescription[PIN_WIRE1_SDA].ulPin,
			g_APinDescription[PIN_WIRE1_SDA].ulPinConfiguration);
	PIO_Configure(
			g_APinDescription[PIN_WIRE1_SCL].pPort,
			g_APinDescription[PIN_WIRE1_SCL].ulPinType,
			g_APinDescription[PIN_WIRE1_SCL].ulPin,
			g_APinDescription[PIN_WIRE1_SCL].ulPinConfiguration);

	NVIC_DisableIRQ(WIRE1_ISR_ID);
	NVIC_ClearPendingIRQ(WIRE1_ISR_ID);
	NVIC_SetPriority(WIRE1_ISR_ID, 0);
	NVIC_EnableIRQ(WIRE1_ISR_ID);
}

TwoWire Wire1 = TwoWire(WIRE1_INTERFACE, Wire1_Init);

void WIRE1_ISR_HANDLER(void) {
	Wire1.onService();
}*/
