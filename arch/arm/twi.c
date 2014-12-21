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

#define twi0_wait_for_tx_complete() while(!TWI_ByteSent(TWI0))
#define twi0_wait_for_rx_complete() while(!TWI_ByteReceived(TWI0))

void twi0_init_default(){
	pmc_enable_periph_clk(ID_PIOA);
  pmc_enable_periph_clk(ID_TWI0);

	PIO_Configure(PIOA, PIO_PERIPH_A, 
		(PIO_PA18A_TWCK0|PIO_PA17A_TWD0), PIO_DEFAULT); 
	
	NVIC_DisableIRQ(TWI0_IRQn);
	NVIC_ClearPendingIRQ(TWI0_IRQn);
	NVIC_SetPriority(TWI0_IRQn, 0);
	NVIC_EnableIRQ(TWI0_IRQn);

	TWI_ConfigureMaster(TWI0, 100000L, SystemCoreClock); 
}

/// address is the first byte of data
void 		twi0_start_write(uint8_t addr, uint8_t *data, uint8_t data_sz){
	TWI_StartWrite(TWI0, addr, 0, 0, data[0]);
	twi0_wait_for_tx_complete(); 
	for(int c = 1; c < data_sz; c++){
		TWI_WriteByte(TWI0, data[c]);
		twi0_wait_for_tx_complete(); 
	}
}

/// address is the first byte of data
void 		twi0_start_read(uint8_t addr, uint8_t *data, uint8_t data_sz){
	TWI_StartRead(TWI0, addr, 0, 0);
	twi0_wait_for_rx_complete(); 
	for(int c = 0; c < data_sz; c++){
		data[c] = TWI_ReadByte(TWI0);
		twi0_wait_for_rx_complete(); 
	}
}

/// sends stop signal on the bus
void twi0_stop(void){
	TWI_SendSTOPCondition(TWI0);
}

/// returns 1 if twi bus is processing another transaction
uint8_t twi0_busy(void){
	return TWI_TransferComplete(TWI0);
}
