#include <arch/soc.h>

#include <kernel/cbuf.h>
#include <kernel/thread.h>

#include <inttypes.h>

#include "twi.h"

#define I2C_SPEED 100000UL

//#define I2C_DEBUG(...) printf(__VA_ARGS__)
#define I2C_DEBUG(...) do {} while(0)

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
	struct cbuf tx_buffer, rx_buffer; 
	uint8_t addr; 
	uint8_t size; 
	i2cdev_status_t flags; 
	struct pt *user_thread; 
	struct libk_thread thread; 
}; 

#define I2C_TX_BUFFER_SIZE 64
#define I2C_RX_BUFFER_SIZE 64
#define I2C_DEV_COUNT (sizeof(_devices) / sizeof(_devices[0]))

static uint8_t _rx_buffers[I2C_DEV_COUNT][I2C_TX_BUFFER_SIZE]; 
static uint8_t _tx_buffers[I2C_DEV_COUNT][I2C_TX_BUFFER_SIZE]; 
static struct i2c_device_data _data[I2C_DEV_COUNT]; 

static I2C_InitTypeDef  I2C_InitStructure; 

static PT_THREAD(_i2c_thread(struct libk_thread *kthread, struct pt *pt)){
	struct i2c_device_data *data = container_of(kthread, struct i2c_device_data, thread); 
	const struct i2c_device *conf = &_devices[data->dev_id]; 
	
	PT_BEGIN(pt); 
	
	while(1){
		// wait for a new request
		PT_WAIT_UNTIL(pt, data->flags & (I2CDEV_READ | I2CDEV_WRITE)); 
		
		while(1){
			//PT_WAIT_WHILE(pt, I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY));
			
			I2C_DEBUG("I2C: start\n"); 
			//PT_WAIT_WHILE(pt, I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY)); 
			I2C_GenerateSTART(conf->dev, ENABLE);

			PT_WAIT_UNTIL(pt, I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_MODE_SELECT));
			
			if(data->flags & I2CDEV_READ){
				I2C_Send7bitAddress(conf->dev, data->addr, I2C_Direction_Receiver);
				PT_WAIT_UNTIL(pt, I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
			} else if(data->flags & I2CDEV_WRITE){
				I2C_Send7bitAddress(conf->dev, data->addr, I2C_Direction_Transmitter);
				PT_WAIT_UNTIL(pt, I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
			} 
			
			I2C_Cmd(conf->dev, ENABLE);
			
			data->flags &= ~I2CDEV_START; 
			
			while(1){
				if(data->flags & (I2CDEV_START | I2CDEV_STOP)){
					// if another start is requested then we break out and send a new start condition
					break; 
				}
				if(data->flags & I2CDEV_READ){
					I2C_DEBUG("I2C: reading %d bytes\n", data->size); 
					
					// read op
					while((data->flags & I2CDEV_READ) && data->size > 0){
						// this is necessary evil because if we don't busy wait, we lose data here because
						// we are not interrupt driven :/ .. yet :)
						BLOCKING_WAIT_WHILE(!I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
						cbuf_put(&data->rx_buffer, I2C_ReceiveData(conf->dev));
						
						data->size--;
						
						if(data->size == 1){
							// last byte: disable ack and send stop
							I2C_AcknowledgeConfig(conf->dev, DISABLE);
							//I2C_GenerateSTOP(conf->dev, ENABLE);
						} 
					}
					
					// reenable ack
					I2C_AcknowledgeConfig(conf->dev, ENABLE);
					
					I2C_DEBUG("I2C: read completed\n"); 
					PT_WAIT_UNTIL(pt, data->flags & (I2CDEV_START | I2CDEV_STOP)); 
					//PT_WAIT_WHILE(pt, data->flags & I2CDEV_RX_NOT_EMPTY); 
					//conf->flags |= I2CDEV_READY; 
					
				} else if(data->flags & I2CDEV_WRITE){
					// write op - wait for more data while stop bit is not set
					// also always make sure we keep going while there is data to be written. 
					I2C_DEBUG("I2C: transmitting %d bytes\n", cbuf_get_waiting(&data->tx_buffer)); 
					while(data->flags & I2CDEV_WRITE){
						if(cbuf_get_waiting(&data->tx_buffer) > 0){
							I2C_SendData(conf->dev, cbuf_get(&data->tx_buffer));  
							PT_WAIT_UNTIL(pt, I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
						} else {
							data->flags |= I2CDEV_READY; 
						}
						PT_YIELD(pt); 
					}
					I2C_DEBUG("I2C: tx completed!\n"); 
					PT_WAIT_UNTIL(pt, data->flags & (I2CDEV_START | I2CDEV_STOP)); 
				} 
				
				PT_YIELD(pt); 
			}
			// if it is no more read or write then we just break
			if(data->flags & I2CDEV_STOP){
				break; 
			}
		}
		I2C_DEBUG("I2C: sending stop\n"); 
	
		I2C_GenerateSTOP(conf->dev, ENABLE);
		
		// wait until stop has been completed 
		//PT_WAIT_WHILE(pt, I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY)); 
		
		// reset all flags 
		// (this also unlocks the device, but never rely on this to happen!)
		data->flags = 0; 
		
		I2C_DEBUG("I2C: bus released!\n"); 
	}
	PT_END(pt); 
}

int8_t i2cdev_init(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = &_data[dev_id]; 
	
	cbuf_init(&data->rx_buffer, _rx_buffers[dev_id], I2C_RX_BUFFER_SIZE); 
	cbuf_init(&data->tx_buffer, _tx_buffers[dev_id], I2C_TX_BUFFER_SIZE); 
	
	data->dev_id = dev_id; 
  data->flags = 0; 
  
	RCC_APB2PeriphClockCmd (conf->rcc_gpio, ENABLE); 
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
  
  // register device threads
  libk_create_thread(&data->thread, _i2c_thread, "i2c"); 
  return 0; 
}

/*
static void i2cdev_wait(uint8_t dev_id, uint8_t addr){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return; 
	I2C_TypeDef *dev = _devices[dev_id].dev; 

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
*/

void i2cdev_deinit(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return; 
	const struct i2c_device *conf = &_devices[dev_id]; 
	I2C_DeInit(conf->dev); 
	
  _data[dev_id].flags = 0; 
}

uint8_t i2cdev_open(uint8_t dev_id){
	if(dev_id >= I2C_DEV_COUNT) return -1; 
	//const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = &_data[dev_id]; 
	
	if(data->flags & I2CDEV_LOCKED) return 0; 
	data->user_thread = libk_current_thread(); 
	data->flags |= I2CDEV_LOCKED; 
	
	// clear all buffers for each new open request
	cbuf_clear(&data->rx_buffer); 
	cbuf_clear(&data->tx_buffer); 
	
	return 1; 
}

void i2cdev_close(uint8_t dev_id){
	if(dev_id >= I2C_DEV_COUNT) return; 
	struct i2c_device_data *data = &_data[dev_id]; 
	// (data->flags & I2CDEV_BUSY) || 
	if(data->user_thread != libk_current_thread()) return; 
	
	// same as stop
	i2cdev_stop(dev_id); 
}

/**
Write operation can only procede if no read is in progress. If device is already 
busy doing a write, then we can add more data into the write buffer to be sent
to the device. Write is stopped by issuing i2cdev_stop()
**/
int16_t i2cdev_write(uint8_t dev_id, uint8_t addr, const uint8_t *buffer, uint8_t bytes_to_send){
	if(dev_id >= I2C_DEV_COUNT) return -1; 
	struct i2c_device_data *data = &_data[dev_id]; 
	
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// if a write is already in progress then we just add more data and exit
	if((data->flags & I2CDEV_WRITE) && (data->flags & I2CDEV_BUSY)){
		I2C_DEBUG("I2C: write init\n"); 
		return cbuf_putn(&data->tx_buffer, buffer, bytes_to_send); 
	}
	
	// otherwise we need to start a write
	data->flags &= ~(I2CDEV_READ | I2CDEV_READY); 
	data->flags |= I2CDEV_WRITE | I2CDEV_BUSY | I2CDEV_START; 
	
	data->addr = addr; 
	
	return cbuf_putn(&data->tx_buffer, buffer, bytes_to_send); 
}

int16_t i2cdev_read(uint8_t dev_id, uint8_t addr, uint8_t *buffer, uint8_t size){
	if(dev_id >= I2C_DEV_COUNT) return -1; 
	//const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = &_data[dev_id]; 
	
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// if we are already reading then we just read the bytes in the read buffer 
	if((data->flags & I2CDEV_READ) && (data->flags & I2CDEV_BUSY)){
		I2C_DEBUG("I2C: read: %d bytes in buffer\n", cbuf_get_waiting(&data->rx_buffer)); 
	
		return cbuf_getn(&data->rx_buffer, buffer, size); 
	}
	
	// otherwise we start a new read
	data->flags &= ~(I2CDEV_WRITE | I2CDEV_READY); 
	data->flags |= (I2CDEV_BUSY | I2CDEV_READ | I2CDEV_START); 
	data->addr = addr; 
	data->size = size; 
	
	return 0; 
}

int8_t i2cdev_stop(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	struct i2c_device_data *data = &_data[dev_id]; 
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// abort any read or write
	data->flags &= ~(I2CDEV_READ | I2CDEV_WRITE); 
	data->flags |= I2CDEV_STOP; 
	
  return 0; 
}

uint8_t i2cdev_status(uint8_t dev_id, i2cdev_status_t flags){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return 0; 
	// we return true only if ALL supplied flags are set
	return (_data[dev_id].flags & flags) == flags; 
}
