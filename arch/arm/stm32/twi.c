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

struct twi_device {
	I2C_TypeDef *dev; 
	GPIO_TypeDef *gpio; 
	int rcc_gpio; 
	int rcc_id; 
	int apb_id; 
	int pins; 
	/*
	DMA_InitTypeDef dma;
	uint8_t read_write; 
	uint16_t addr; 
} _twi_dev[] = {
	{
		.dma = {0}, 
		.read_write = 1, 
		.addr = 0
	}*/
}; 

static const struct twi_device _devices[] = {
	{
		.dev = I2C1, 
		.gpio = GPIOB, 
		.rcc_gpio = RCC_APB2Periph_GPIOB, 
		.rcc_id = RCC_APB1Periph_I2C1, 
		.apb_id = 1, 
		.pins = GPIO_Pin_7 | GPIO_Pin_6
	}, 
	{
		.dev = I2C2, 
		.gpio = GPIOB, 
		.rcc_gpio = RCC_APB2Periph_GPIOB, 
		.rcc_id = RCC_APB1Periph_I2C2, 
		.apb_id = 1, 
		.pins = GPIO_Pin_10 | GPIO_Pin_11
	}
}; 

#define WAIT(expr) do {} while(expr) //{uint32_t timeout = 10; while((expr) && timeout--) delay_ms(1);}

static I2C_InitTypeDef  I2C_InitStructure; 

int8_t twi_init(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	const struct twi_device *conf = &_devices[dev_id]; 

	RCC_APB2PeriphClockCmd ( conf->rcc_gpio, ENABLE); 
	RCC_APB1PeriphClockCmd(conf->rcc_id, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
  GPIO_InitTypeDef  GPIO_InitStructure; 

  /* Configure I2C1 pins: SCL and SDA */
  GPIO_InitStructure.GPIO_Pin =  conf->pins;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(conf->gpio, &GPIO_InitStructure);

  /* I2C configuration */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0xa0;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  
  /* I2C Peripheral Enable */
  I2C_Cmd(conf->dev, ENABLE);
  /* Apply I2C configuration after enabling it */
  I2C_Init(conf->dev, &I2C_InitStructure);
  
  return 0; 
}

void twi_deinit(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return; 
	const struct twi_device *conf = &_devices[dev_id]; 
	I2C_DeInit(conf->dev); 
}

int8_t twi_start_write(uint8_t dev_id, uint8_t adr, const uint8_t *data, uint8_t bytes_to_send){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	const struct twi_device *conf = &_devices[dev_id]; 

	/* While the bus is busy */
	WAIT(I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY));
	/* Send START condition */
	I2C_GenerateSTART(conf->dev, ENABLE);

	/* Test on EV5 and clear it */
	WAIT(!I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_MODE_SELECT));
	/* Send EEPROM address for write */
	I2C_Send7bitAddress(conf->dev, adr, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	WAIT(!I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	/* Clear EV6 by setting again the PE bit */
	I2C_Cmd(conf->dev, ENABLE);

	for(unsigned c = 0; c < bytes_to_send; c++){
		/* Send the EEPROM's internal address to write to */
		I2C_SendData(conf->dev, *data);  
		
		data++; 
		
		/* Test on EV8 and clear it */
		WAIT(!I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	return 0; 
}

int8_t twi_start_read(uint8_t dev_id, uint8_t adr, uint8_t *data, uint8_t bytes_to_read){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	I2C_TypeDef *dev = _devices[dev_id].dev; 
	
	/* Send STRAT condition a second time */  
	I2C_GenerateSTART(dev, ENABLE);

	/* Test on EV5 and clear it */
	WAIT(!I2C_CheckEvent(dev, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send EEPROM address for read */
	I2C_Send7bitAddress(dev, adr, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */
	WAIT(!I2C_CheckEvent(dev, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	/* Clear EV6 by setting again the PE bit */
	I2C_Cmd(dev, ENABLE);

	/* While there is data to be read */
	//uint32_t timeout = 10; 
	while(bytes_to_read){
		if(bytes_to_read == 1)
		{
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(dev, DISABLE);
			
			/* Send STOP Condition */
			I2C_GenerateSTOP(dev, ENABLE);
		}

		/* Test on EV7 and clear it */
		if(I2C_CheckEvent(dev, I2C_EVENT_MASTER_BYTE_RECEIVED))  
		{      
			/* Read a byte from the EEPROM */
			*data = I2C_ReceiveData(dev);

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
	I2C_AcknowledgeConfig(dev, ENABLE);
	
	return 0; 
}

int8_t twi_stop(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	I2C_TypeDef *dev = _devices[dev_id].dev; 

	I2C_GenerateSTOP(dev, ENABLE);
	//I2C_Cmd(I2C1, DISABLE);
	
  return 0; 
}

void twi_wait(uint8_t dev_id, uint8_t addr){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return; 
	I2C_TypeDef *dev = _devices[dev_id].dev; 

	__IO uint16_t SR1_Tmp = 0;

	do
	{
		/* Send START condition */
		I2C_GenerateSTART(dev, ENABLE);
		/* Read I2C1 SR1 register */
		SR1_Tmp = I2C_ReadRegister(dev, I2C_Register_SR1); 
		/* Send EEPROM address for write */
		I2C_Send7bitAddress(dev, addr, I2C_Direction_Transmitter);
	}while(!(I2C_ReadRegister(dev, I2C_Register_SR1) & 0x0002));

	(void)(SR1_Tmp); 

	/* Clear AF flag */
	I2C_ClearFlag(dev, I2C_FLAG_AF);

	/* STOP condition */    
	I2C_GenerateSTOP(dev, ENABLE);  
}
