#include <arch/soc.h>
#include "twi.h"

void twi0_init_default(){
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; 
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; 
	AFIO->MAPR &= ~AFIO_MAPR_I2C1_REMAP; 
	
	//RCC_APB1PeriphClockCmd(HMC5883L_I2C_RCC_Periph, ENABLE);
  //RCC_APB2PeriphClockCmd(HMC5883L_I2C_RCC_Port, ENABLE);
  
	GPIO_InitTypeDef  GPIO_InitStructure;

  /* Configure I2C1 pins: SCL and SDA */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  I2C_InitTypeDef  I2C_InitStructure;

  /* I2C configuration */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  //I2C_InitStructure.I2C_OwnAddress1 = I2C1_SLAVE_ADDRESS7;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = 100000L;
 
  /* I2C Peripheral Enable */
  I2C_Cmd(I2C1, ENABLE);
  /* Apply I2C configuration after enabling it */
  I2C_Init(I2C1, &I2C_InitStructure);
}

void twi0_start_write(uint8_t adr, uint8_t *data, uint8_t bytes_to_send){
	/* Send STRAT condition */
  I2C_GenerateSTART(I2C1, ENABLE);

  /* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  

  /* Send EEPROM address for write */
  I2C_Send7bitAddress(I2C1, adr, I2C_Direction_Transmitter);
 
  /* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
     
  /* Send the EEPROM's internal address to write to */
  for(int c = 0; c < bytes_to_send; c++){
		I2C_SendData(I2C1, data[c]);
		/* Test on EV8 and clear it */
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
}

void twi0_start_read(uint8_t adr, uint8_t *data, uint8_t bytes_to_read){
	/* Send START condition */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send EEPROM address for read */
	I2C_Send7bitAddress(I2C1, adr, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	/* While there is data to be read */
	while(bytes_to_read)  
	{
		if(bytes_to_read == 1)
		{
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2C1, DISABLE);
		}

		/* Test on EV7 and clear it */
		if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  
		{
			*data = I2C_ReceiveData(I2C1);
			data++;
		 
			bytes_to_read--;        
		}  
	}

	/* Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(I2C1, ENABLE);
}

int16_t twi0_stop(void){
  /* Send STOP condition */
  I2C_GenerateSTOP(I2C1, ENABLE);
  return 0; 
}
