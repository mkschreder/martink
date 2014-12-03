/*
bh1750 lib 0x02

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <util/delay.h>

#include "bh1750.h"

#include <i2c.h>

/*
 * init bh1750
 */
void bh1750_init(void) {

	#if BH1750_I2CINIT == 1
	//init i2c
	i2c_init();
	_delay_us(10);
	#endif

	//write config
	i2c_start_wait(BH1750_ADDR | I2C_WRITE);
	i2c_write(BH1750_MODE);
	i2c_stop();
}

/*
 * read lux value
 */
uint16_t bh1750_getlux(void) {
	uint16_t ret = 0;

	i2c_start_wait(BH1750_ADDR | I2C_READ);
	ret = i2c_readAck();
	ret <<= 8;
	ret |= i2c_readNak();
	i2c_stop();

	ret = ret/1.2;
	return ret;
}
