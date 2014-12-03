/*
wiinunchuck 0x02

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <string.h>
#include <arch/soc.h>

#include "wiinunchuck.h"

#if WIINUNCHUCK_GETATTITUDE == 1
#include <math.h>  //include libm
#endif


volatile uint8_t wiinunchuck_joyX = 0;
volatile uint8_t wiinunchuck_joyY = 0;
#if WIINUNCHUCK_PULSEBUTTON == 1
volatile uint8_t wiinunchuck_lastbuttonZ = 0;
volatile uint8_t wiinunchuck_lastbuttonC = 0;
#endif
volatile uint8_t wiinunchuck_buttonZ = 0;
volatile uint8_t wiinunchuck_buttonC = 0;
volatile int wiinunchuck_angleX = 0;
volatile int wiinunchuck_angleY = 0;
volatile int wiinunchuck_angleZ = 0;
#if WIINUNCHUCK_ANGLEFILTER == 1
volatile int wiinunchuck_avarageangleX[WIINUNCHUCK_ANGLEAVARAGECOEF];
volatile int wiinunchuck_avarageangleY[WIINUNCHUCK_ANGLEAVARAGECOEF];
volatile int wiinunchuck_avarageangleZ[WIINUNCHUCK_ANGLEAVARAGECOEF];
#endif

/*
 * get joypad X
 */
int wiinunchuck_getjoyX(void) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1

	return (int)wiinunchuck_joyX;

	#else

	#if WIINUNCHUCK_JOYCENTERB == 1
	int joyX = (int)wiinunchuck_joyX;
	if(joyX <= WIINUNCHUCK_CENTERJOYX)
		joyX = (joyX - WIINUNCHUCK_INMINJOYX) * (127 - 0) / (WIINUNCHUCK_CENTERJOYX - WIINUNCHUCK_INMINJOYX) + 0;
	else
		joyX = (joyX - WIINUNCHUCK_CENTERJOYX) * (255 - 127) / (WIINUNCHUCK_INMAXJOYX - WIINUNCHUCK_CENTERJOYX) + 127 - 1;
	return joyX-127;
	#else
	return (int)wiinunchuck_joyX - WIINUNCHUCK_DEFAULTZEROJOYX;
	#endif

	#endif
}

/*
 * get joypad Y
 */
int wiinunchuck_getjoyY(void) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1

	return (int)wiinunchuck_joyY;

	#else

	#if WIINUNCHUCK_JOYCENTERB == 1
	int joyY = (int)wiinunchuck_joyY;
	if(joyY <= WIINUNCHUCK_CENTERJOYY)
		joyY = (joyY - WIINUNCHUCK_INMINJOYY) * (127 - 0) / (WIINUNCHUCK_CENTERJOYY - WIINUNCHUCK_INMINJOYY) + 0;
	else
		joyY = (joyY - WIINUNCHUCK_CENTERJOYY) * (255 - 127) / (WIINUNCHUCK_INMAXJOYY - WIINUNCHUCK_CENTERJOYY) + 127 - 1;
	return joyY-127;
	#else
	return (int)wiinunchuck_joyY - WIINUNCHUCK_DEFAULTZEROJOYY;
	#endif

	#endif
}

/*
 * get button Z
 */
uint8_t wiinunchuck_getbuttonZ(void) {
	#if WIINUNCHUCK_PULSEBUTTON == 1
	return (wiinunchuck_buttonZ && !wiinunchuck_lastbuttonZ);
	#else
	return wiinunchuck_buttonZ;
	#endif
}

/*
 * get button C
 */
uint8_t wiinunchuck_getbuttonC(void) {
	#if WIINUNCHUCK_PULSEBUTTON == 1
	return (wiinunchuck_buttonC && !wiinunchuck_lastbuttonC);
	#else
	return wiinunchuck_buttonC;
	#endif
}

/*
 * get angle X
 */
int wiinunchuck_getangleX(void) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1
	return wiinunchuck_angleX;
	#else
	return wiinunchuck_angleX - WIINUNCHUCK_ZEROANGLEX;
	#endif
}

/*
 * get angle Y
 */
int wiinunchuck_getangleY(void) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1
	return wiinunchuck_angleY;
	#else
	return wiinunchuck_angleY - WIINUNCHUCK_ZEROANGLEY;
	#endif
}

/*
 * get angle Z
 */
int wiinunchuck_getangleZ(void) {
	return wiinunchuck_angleZ - WIINUNCHUCK_ZEROANGLEZ;
}

#if WIINUNCHUCK_GETATTITUDE == 1
/*
 * estimate pitch and row using euleros angles
 */
void wiinunchuck_getpitchroll(double ax, double ay, double az, double *pitch, double *roll) {
	double magnitude = sqrt(((uint32_t)ax * (uint32_t)ax) + ((uint32_t)ay * (uint32_t)ay) + ((uint32_t)az * (uint32_t)az));
	ax = ax / magnitude;
	ay = ay / magnitude;
	az = az / magnitude;
	*pitch = atan2(ax, sqrt(pow(ay,2) + pow(az, 2)));
	*roll = atan2(ay, az);
}
#endif

#if WIINUNCHUCK_ANGLEFILTER == 1
/*
 * avarage filter over an array
 */
int wiinunchuck_avaragefilter(int input, int wiinunchuck_avarageangle[]) {
	uint8_t i=0;
	long sum=0;
	for (i=0; i<WIINUNCHUCK_ANGLEAVARAGECOEF; i++) {
		wiinunchuck_avarageangle[i] = wiinunchuck_avarageangle[i+1];
	}
	wiinunchuck_avarageangle[WIINUNCHUCK_ANGLEAVARAGECOEF-1] = input;
	for (i=0; i<WIINUNCHUCK_ANGLEAVARAGECOEF; i++) {
		sum += wiinunchuck_avarageangle[i];
	}
	return (sum/WIINUNCHUCK_ANGLEAVARAGECOEF);
}
#endif

/*
 * get new data
 */
void wiinunchuck_update() {
	uint8_t i=0;
	uint8_t buff[WIINUNCHUCK_READBYTES];
	memset(buff, 0, sizeof(buff));

	//request data
	i2c_start_wait(WIINUNCHUCK_ADDR | I2C_WRITE);
	i2c_write(0x00);
	i2c_stop();
	i2c_start_wait(WIINUNCHUCK_ADDR | I2C_READ);
	for(i=0; i<WIINUNCHUCK_READBYTES; i++) {
		if(i==WIINUNCHUCK_READBYTES-1)
			buff[i] = i2c_readNak();
		else
			buff[i] = i2c_readAck();
	}
	i2c_stop();

	//decodebyte
	#if WIINUNCHUCK_DECODEBYTE == 1
	for(i=0; i<WIINUNCHUCK_READBYTES; i++) {
		buff[i] = wiinunchuck_decode(buff[i]);
	}
	#endif

	//record old records
	#if WIINUNCHUCK_PULSEBUTTON == 1
	wiinunchuck_lastbuttonZ = wiinunchuck_buttonZ;
	wiinunchuck_lastbuttonC = wiinunchuck_buttonC;
	#endif

	//get joypad
	wiinunchuck_joyX = buff[0];
	wiinunchuck_joyY = buff[1];

	//get button
	wiinunchuck_buttonZ = !(buff[5] & 0b00000001);
	wiinunchuck_buttonC = !((buff[5] & 0b00000010) >> 1);

	//get angle
	wiinunchuck_angleX = (buff[2] << 2) + ((buff[5] & (0b00000011 << (1*2)) >> (1*2)));
	wiinunchuck_angleY = (buff[3] << 2) + ((buff[5] & (0b00000011 << (2*2)) >> (2*2)));
	wiinunchuck_angleZ = (buff[4] << 2) + ((buff[5] & (0b00000011 << (3*2)) >> (3*2)));

	//filter angle
	#if WIINUNCHUCK_ANGLEFILTER == 1
	wiinunchuck_angleX = wiinunchuck_avaragefilter(wiinunchuck_angleX, (int *)wiinunchuck_avarageangleX);
	wiinunchuck_angleY = wiinunchuck_avaragefilter(wiinunchuck_angleY, (int *)wiinunchuck_avarageangleY);
	wiinunchuck_angleZ = wiinunchuck_avaragefilter(wiinunchuck_angleZ, (int *)wiinunchuck_avarageangleZ);
	#endif
}


/*
 * init wiinunchuck
 */
void wiinunchuck_init(void) {
	#if WIINUNCHUCK_I2CINIT == 1
	//init i2c
	i2c_init();
	_delay_us(100);
	#endif

	//standard init: 0x40 -> 0x00
	//alternative init: 0xF0 -> 0x55 followed by 0xFB -> 0x00, lets us use 3rd party nunchucks
	//no longer need to decode bytes in _nunchuk_decode_byte
	i2c_start_wait(WIINUNCHUCK_ADDR | I2C_WRITE);
	i2c_write(0xF0);
	i2c_write(0x55);
	i2c_stop();
	i2c_start_wait(WIINUNCHUCK_ADDR | I2C_WRITE);
	i2c_write(0xFB);
	i2c_write(0x00);
	i2c_stop();
	//update
	wiinunchuck_update();
}




