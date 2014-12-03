/**********************************************************

Software I2C Library for AVR Devices.

Copyright 2008-2012
eXtreme Electronics, India
www.eXtremeElectronics.co.in
**********************************************************/
 
#include <avr/io.h>
#include <util/delay.h>

#include "i2csoft.h"

#define Q_DEL _delay_loop_2(3)
#define H_DEL _delay_loop_2(5)

void SoftI2CInit(void)
{
	SDAPORT&=(1<<SDA);
	SCLPORT&=(1<<SCL);
	
	SOFT_I2C_SDA_HIGH;	
	SOFT_I2C_SCL_HIGH;	
		
}
void SoftI2CStart(void)
{
	SOFT_I2C_SCL_HIGH;
	H_DEL;
	
	SOFT_I2C_SDA_LOW;	
	H_DEL;  	
}

void SoftI2CStop(void)
{
	 SOFT_I2C_SDA_LOW;
	 H_DEL;
	 SOFT_I2C_SCL_HIGH;
	 Q_DEL;
	 SOFT_I2C_SDA_HIGH;
	 H_DEL;
}

uint8_t SoftI2CWriteByte(uint8_t data)
{
	 
	 uint8_t i;
	 	
	 for(i=0;i<8;i++)
	 {
		SOFT_I2C_SCL_LOW;
		Q_DEL;
		
		if(data & 0x80)
			SOFT_I2C_SDA_HIGH;
		else
			SOFT_I2C_SDA_LOW;	
		
		H_DEL;
		
		SOFT_I2C_SCL_HIGH;
		H_DEL;
		
		while((SCLPIN & (1<<SCL))==0);
			
		data=data<<1;
	}
	 
	//The 9th clock (ACK Phase)
	SOFT_I2C_SCL_LOW;
	Q_DEL;
		
	SOFT_I2C_SDA_HIGH;		
	H_DEL;
		
	SOFT_I2C_SCL_HIGH;
	H_DEL;	
	
	uint8_t ack=!(SDAPIN & (1<<SDA));
	
	SOFT_I2C_SCL_LOW;
	H_DEL;
	
	return ack;
	 
}
 
 
uint8_t SoftI2CReadByte(uint8_t ack)
{
	uint8_t data=0x00;
	uint8_t i;
			
	for(i=0;i<8;i++)
	{
			
		SOFT_I2C_SCL_LOW;
		H_DEL;
		SOFT_I2C_SCL_HIGH;
		H_DEL;
			
		while((SCLPIN & (1<<SCL))==0);
		
		if(SDAPIN &(1<<SDA))
			data|=(0x80>>i);
			
	}
		
	SOFT_I2C_SCL_LOW;
	Q_DEL;						//Soft_I2C_Put_Ack
	
	if(ack)
	{
		SOFT_I2C_SDA_LOW;	
	}
	else
	{
		SOFT_I2C_SDA_HIGH;
	}	
	H_DEL;
	
	SOFT_I2C_SCL_HIGH;
	H_DEL;
	
	SOFT_I2C_SCL_LOW;
	H_DEL;
			
	return data;
	
}















