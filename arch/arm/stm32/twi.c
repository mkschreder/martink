#include <arch/soc.h>

#include <util/cbuf.h>
#include <thread/pt.h>

#include <inttypes.h>

#include "twi.h"

#define I2C_SPEED 100000UL

#define I2C_DEBUG(...) do {} while(0) // printf(__VA_ARGS__)

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

#define I2C_TX_BUFFER_SIZE 32
#define I2C_RX_BUFFER_SIZE 32
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
		//PT_WAIT_WHILE(pt, I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY));
		
		I2C_DEBUG("I2C: waiting for command\n"); 
		
		// busy is set in read/write methods. wait for read or a write
		PT_WAIT_UNTIL(pt, (data->flags & I2CDEV_BUSY)); 
		
		while(1){
			if(data->flags & (I2CDEV_READ | I2CDEV_WRITE)){
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
				
				if(data->flags & I2CDEV_READ){
					I2C_DEBUG("I2C: reading %d bytes\n", data->size); 
					// read op
					while(data->size){
						// ensure that there is space in the buffer
						PT_WAIT_UNTIL(pt, cbuf_get_free(&data->rx_buffer) > 0); 
						
						if(data->size == 1){
							// last byte: disable ack and send stop
							I2C_AcknowledgeConfig(conf->dev, DISABLE);
							//I2C_GenerateSTOP(dev, ENABLE);
						}

						PT_WAIT_UNTIL(pt, I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
						
						cbuf_put(&data->rx_buffer, I2C_ReceiveData(conf->dev));
						
						data->flags |= I2CDEV_RX_NOT_EMPTY; 
						data->size--;
					}
					
					// reenable ack
					I2C_AcknowledgeConfig(conf->dev, ENABLE);
					
					I2C_DEBUG("I2C: read completed\n"); 
					
					data->flags &= ~I2CDEV_READ; 
					// wait while read flag is set. it should be reset using i2cdev_stop() or another write. 
					//PT_WAIT_WHILE(pt, data->flags & I2CDEV_READ); 
				} else if(data->flags & I2CDEV_WRITE){
					// write op - wait for more data while stop bit is not set
					// also always make sure we keep going while there is data to be written. 
					I2C_DEBUG("I2C: transmitting %d bytes\n", cbuf_get_waiting(&data->tx_buffer)); 
					while(!(data->flags & I2CDEV_STOP) && cbuf_get_waiting(&data->tx_buffer) > 0){
						PT_WAIT_UNTIL(pt, cbuf_get_waiting(&data->tx_buffer) > 0); 
						I2C_SendData(conf->dev, cbuf_get(&data->tx_buffer));  
						PT_WAIT_UNTIL(pt, I2C_CheckEvent(conf->dev, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
					}
					I2C_DEBUG("I2C: tx completed!\n"); 
					data->flags |= I2CDEV_TX_DONE; 
					data->flags &= ~I2CDEV_WRITE; 
				} 
			} else if(data->flags & I2CDEV_STOP){
				I2C_DEBUG("I2C: sending stop\n"); 
				
				I2C_GenerateSTOP(conf->dev, ENABLE);
				
				// wait until stop has been completed 
				PT_WAIT_WHILE(pt, I2C_GetFlagStatus(conf->dev, I2C_FLAG_BUSY)); 
				
				// reset all flags 
				// (this also unlocks the device, but never rely on this to happen!)
				data->flags = 0; 
				
				break; // exit loop
			}
			PT_YIELD(pt); 
		}
		//data->status &= ~(I2CDEV_BUSY); 
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
	
	// close must always make sure that the current operation completes first
	data->flags |= I2CDEV_STOP; 
}

/**
Write operation can only procede if no read is in progress. If device is already 
busy doing a write, then we can add more data into the write buffer to be sent
to the device. Write is stopped by issuing i2cdev_stop()
**/
int16_t i2cdev_write(uint8_t dev_id, uint8_t addr, const uint8_t *buffer, uint8_t bytes_to_send){
	if(dev_id >= I2C_DEV_COUNT) return -1; 
	//const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = &_data[dev_id]; 
	uint8_t written = 0; 
	
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// if read operation was completed then it is safe to do another write
	//if(data->flags & I2CDEV_RX_DONE) data->flags &= ~(I2CDEV_RX_DONE | I2CDEV_READ); 
	
	// write is invalid while a read is in progress 
	if(data->flags & I2CDEV_READ) return -1; 
	
	data->flags &= ~I2CDEV_TX_DONE; // reset tx done flag
	
	if(!(data->flags & I2CDEV_WRITE)){
		data->flags |= I2CDEV_BUSY | I2CDEV_WRITE; // signal busy and write in progress
		data->addr = addr; 
	}
	
	written = cbuf_putn(&data->tx_buffer, buffer, bytes_to_send); 
	
	I2C_DEBUG("I2C: put %d bytes\n", written); 
	
	return written; 
}

int16_t i2cdev_read(uint8_t dev_id, uint8_t addr, uint8_t *buffer, uint8_t size){
	if(dev_id >= I2C_DEV_COUNT) return -1; 
	//const struct i2c_device *conf = &_devices[dev_id]; 
	struct i2c_device_data *data = &_data[dev_id]; 
	
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// if previous write has been completed then we can safely reset write flag
	if(data->flags & I2CDEV_TX_DONE) data->flags &= ~(I2CDEV_TX_DONE | I2CDEV_WRITE); 
	
	// issuing a read while a write is in progress is not allowed
	if(data->flags & I2CDEV_WRITE) return -1; 
	
	// flags are set here as well as number of bytes to be loaded from hardware. 
	// in all subsequent calls, size is only used to read bytes from the rx_buffer. 
	if(!(data->flags & I2CDEV_READ)){
		data->flags |= (I2CDEV_BUSY | I2CDEV_READ); 
		data->size = size; 
		data->addr = addr; 
		return 0; 
	}
	
	uint16_t waiting = cbuf_get_waiting(&data->rx_buffer); 
	
	I2C_DEBUG("I2C: read: %d bytes in buffer\n", waiting); 
	
	// read will always return 0 at first call 
	if(waiting == 0) return 0; 
	
	if(waiting > size) waiting = size; 
	
	cbuf_getn(&data->rx_buffer, buffer, waiting); 
	
	return waiting; 
}

int8_t i2cdev_stop(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	//I2C_TypeDef *dev = _devices[dev_id].dev; 
	struct i2c_device_data *data = &_data[dev_id]; 
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// we need to only set a flag because a previous operation may still be in progress
	data->flags |= I2CDEV_STOP; 
	
  return 0; 
}

uint8_t i2cdev_status(uint8_t dev_id, i2cdev_status_t flags){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	// we return true only if ALL supplied flags are set
	return (_data[dev_id].flags & flags) == flags; 
}
/*
uint8_t i2cdev_open(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return 0; 
	if(_data[dev_id].flags & I2CDEV_LOCKED) return 0;
	_data[dev_id].flags |= I2CDEV_LOCKED; 
	
	// clear all buffers for each new open request
	cbuf_clear(&data->rx_buffer); 
	cbuf_clear(&data->tx_buffer); 
	
	return 1; 
}

int8_t i2cdev_close(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	struct i2c_device_data *data = &_data[dev_id]; 
	if(data->user_thread != libk_current_thread()) return -1; 
	
	// close must always make sure that the current operation completes first
	data->flags |= I2CDEV_STOP; 
}*/

/*
void twi_wait(uint8_t dev_id, uint8_t addr){
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
}*/
