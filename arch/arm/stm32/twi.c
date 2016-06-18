#include <arch/soc.h>

#include <kernel/cbuf.h>
#include <kernel/mt.h>

#include <inttypes.h>

#include "twi.h"

#define I2C_SPEED 100000UL

#define I2C_DEBUG(...) {} //printf(__VA_ARGS__)

struct i2c_device {
	I2C_TypeDef *dev; 
	GPIO_TypeDef *gpio; 
	int rcc_gpio; 
	int rcc_id; 
	int apb_id; 
	int pins; 
}; 

static const struct i2c_device _devices[] = {
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

struct i2c_device_data {
	uint8_t dev_id; 
	uint8_t addr; 
	uint8_t flags; 
	ssize_t offset; 
}; 

#define I2C_FLAG_SEND_STOP (1)

#define I2C_DEV_COUNT (sizeof(_devices) / sizeof(_devices[0]))

static struct i2c_device_data _data[I2C_DEV_COUNT]; 

#if 0
static int8_t i2cdev_init(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = &_data[dev_id]; 
	
	data->dev_id = dev_id; 
	data->offset = 0; 
	data->flags = 0; 
	
	RCC_APB2PeriphClockCmd (conf->rcc_gpio, ENABLE); 
	RCC_APB1PeriphClockCmd(conf->rcc_id, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef  GPIO_InitStructure; 
	I2C_InitTypeDef  I2C_InitStructure; 

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

	// register device threads
	//libk_create_thread(&data->thread, _i2c_thread, "i2c"); 
  return 0; 
}

static void i2cdev_wait(I2C_TypeDef *dev, uint8_t addr){

	__IO uint16_t SR1_Tmp = 0;

	do
	{
		I2C_GenerateSTART(dev, ENABLE);
		SR1_Tmp = I2C_ReadRegister(dev, I2C_Register_SR1); 
		I2C_Send7bitAddress(dev, addr, I2C_Direction_Transmitter);
	}while(!(I2C_ReadRegister(dev, I2C_Register_SR1) & 0x0002));

	(void)(SR1_Tmp); 

	I2C_ClearFlag(dev, I2C_FLAG_AF);

	I2C_GenerateSTOP(dev, ENABLE);  
}

static void i2cdev_deinit(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return; 
	const struct i2c_device *conf = &_devices[dev_id]; 
	I2C_DeInit(conf->dev); 
	
  _data[dev_id].flags = 0; 
}

static int stm32_i2c_write(struct i2c_device *self, const uint8_t *buffer, ssize_t bytes_to_send){
	const struct i2c_device *conf = &_devices[data->dev_id]; 

	I2C_DEBUG("I2C: write\n"); 
	
	I2C_GenerateSTART(conf->dev, ENABLE);
	while(I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(conf->dev, data->addr, I2C_Direction_Transmitter);
	while(I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 
	
	I2C_Cmd(conf->dev, ENABLE);
	
	I2C_DEBUG("I2C: transmitting %d bytes\n", bytes_to_send); 
	while(data->offset < bytes_to_send){
		I2C_SendData(conf->dev, buffer[data->offset++]);  
		while(!I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 
	}
	I2C_DEBUG("I2C: tx completed!\n"); 
	
	if(data->flags & I2C_FLAG_SEND_STOP) {
		I2C_DEBUG("I2C: stop\n"); 
		I2C_GenerateSTOP(conf->dev, ENABLE);
		while(!I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY)); 
		i2cdev_wait(conf->dev, data->addr);
	}
	
	data->offset = 0;
}

static int stm32_i2c_read(struct i2c_adapter *adapter, uint8_t addr, uint8_t *buffer, ssize_t size){
	//if(dev_id >= I2C_DEV_COUNT) return -1; 
	//const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = container_of(self, struct i2c_device_data, io); 
	const struct i2c_device *conf = &_devices[data->dev_id]; 
	
	I2C_DEBUG("I2C: read %d to %d\n", size, data->offset); 
	//i2cdev_wait(conf->dev, data->addr); 
	
	//PT_WAIT_WHILE(pt, I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY)); 
	I2C_GenerateSTART(conf->dev, ENABLE);
	while(I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(conf->dev, data->addr, I2C_Direction_Receiver);
	while(I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
	I2C_Cmd(conf->dev, ENABLE);
			
	I2C_DEBUG("I2C: reading %d bytes\n", size); 
	
	//i2cdev_wait(conf->dev, data->addr); 
	
	// read op
	while(data->offset < size){
		// this is necessary evil because if we don't busy wait, we lose data here because
		// we are not interrupt driven :/ .. yet :)
		while(!I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
		buffer[data->offset++] = I2C_ReceiveData(conf->dev);
		
		if(data->offset == (size - 1)){
			// last byte: disable ack and send stop
			I2C_AcknowledgeConfig(conf->dev, DISABLE);
			//I2C_GenerateSTOP(conf->dev, ENABLE);
		} 
	}
	
	// reenable ack
	I2C_AcknowledgeConfig(conf->dev, ENABLE);
	
	if(data->flags & I2C_FLAG_SEND_STOP) {
		I2C_DEBUG("I2C: stop\n"); 
		I2C_GenerateSTOP(conf->dev, ENABLE);
		while(!I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY)); 
	}
	
	data->offset = 0;
	
	I2C_DEBUG("I2C: read completed\n"); 
}
#endif
