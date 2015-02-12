#include <arch/soc.h>
#include "twi.h"

/*
#define EEPROM_PAGE_SIZE 32
#define EEPROM_PAGE_MASK 0xFFE0

#define I2C_PINS	(1 << 6 | 1 << 7)
#define I2C_SPEED 100000UL

#define EEPROM_ADDR 0xA0

typedef enum _state {
	STATE_IDLE,
	STATE_START,
	STATE_ADDRESSED1,
	STATE_ADDRESSED2,
	STATE_RESTART,
	STATE_TRANSFER_START,
	STATE_TRANSFERRING,
	STATE_COMPLETING,
	STATE_COMPLETE,
	STATE_ERROR
} STATE;

#define PAGE_ALIGN 1
*/

#define I2C_SPEED 100000UL

static volatile struct twi_dev {
	DMA_InitTypeDef dma;
	uint8_t read_write; 
	uint16_t addr; 
} _twi_dev[] = {
	{
		.dma = {0}, 
		.read_write = 1, 
		.addr = 0
	}
}; 

#define WAIT(expr) do {} while(expr) //{uint32_t timeout = 10; while((expr) && timeout--) delay_ms(1);}

static I2C_InitTypeDef  I2C_InitStructure; 

void twi0_init_default(void){
	RCC_APB2PeriphClockCmd ( 
		RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
		RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE); 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
  GPIO_InitTypeDef  GPIO_InitStructure; 

  /* Configure I2C1 pins: SCL and SDA */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* I2C configuration */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0xa0;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  
  /* I2C Peripheral Enable */
  I2C_Cmd(I2C1, ENABLE);
  /* Apply I2C configuration after enabling it */
  I2C_Init(I2C1, &I2C_InitStructure);
}

void twi0_start_write(uint8_t adr, const uint8_t *data, uint8_t bytes_to_send){
	/* While the bus is busy */
	WAIT(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	/* Send START condition */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	WAIT(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	/* Send EEPROM address for write */
	I2C_Send7bitAddress(I2C1, adr, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	WAIT(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	/* Clear EV6 by setting again the PE bit */
	I2C_Cmd(I2C1, ENABLE);

	for(unsigned c = 0; c < bytes_to_send; c++){
		/* Send the EEPROM's internal address to write to */
		I2C_SendData(I2C1, *data);  
		
		data++; 
		
		/* Test on EV8 and clear it */
		WAIT(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
}

void twi0_start_read(uint8_t adr, uint8_t *data, uint8_t bytes_to_read){
	/* Send STRAT condition a second time */  
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	WAIT(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send EEPROM address for read */
	I2C_Send7bitAddress(I2C1, adr, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */
	WAIT(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	/* Clear EV6 by setting again the PE bit */
	I2C_Cmd(I2C1, ENABLE);

	/* While there is data to be read */
	//uint32_t timeout = 10; 
	while(bytes_to_read){
		if(bytes_to_read == 1)
		{
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2C1, DISABLE);
			
			/* Send STOP Condition */
			I2C_GenerateSTOP(I2C1, ENABLE);
		}

		/* Test on EV7 and clear it */
		if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  
		{      
			/* Read a byte from the EEPROM */
			*data = I2C_ReceiveData(I2C1);

			/* Point to the next location where the byte read will be saved */
			data++; 
			
			/* Decrement the read bytes counter */
			bytes_to_read--;  
			
			//timeout = 10;       
		} else {
			/*delay_ms(1); 
			
			timeout--; 
			
			if(timeout == 0) {
				I2C_GenerateSTOP(I2C1, ENABLE);
				break; 
			}*/
		} 
	}

	/* Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(I2C1, ENABLE);
}

int16_t twi0_stop(void){
	I2C_GenerateSTOP(I2C1, ENABLE);
	//I2C_Cmd(I2C1, DISABLE);
	
  return 0; 
}

void twi0_wait(uint8_t addr){
	__IO uint16_t SR1_Tmp = 0;

	do
	{
		/* Send START condition */
		I2C_GenerateSTART(I2C1, ENABLE);
		/* Read I2C1 SR1 register */
		SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1); 
		/* Send EEPROM address for write */
		I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
	}while(!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002));

	(void)(SR1_Tmp); 

	/* Clear AF flag */
	I2C_ClearFlag(I2C1, I2C_FLAG_AF);

	/* STOP condition */    
	I2C_GenerateSTOP(I2C1, ENABLE);  
}
