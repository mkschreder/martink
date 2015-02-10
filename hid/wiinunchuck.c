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

#include <math.h>

//definitions
#define WIINUNCHUCK_ADDR (0x52<<1) //device address
#define WIINUNCHUCK_READBYTES 6

//calibration
#define WIINUNCHUCK_ZEROANGLEX 527
#define WIINUNCHUCK_ZEROANGLEY 531
#define WIINUNCHUCK_ZEROANGLEZ 530
#if WIINUNCHUCK_JOYCENTERB == 1
#define WIINUNCHUCK_CENTERJOYX 137 //the center as read
#define WIINUNCHUCK_INMINJOYX 0 //the min as read
#define WIINUNCHUCK_INMAXJOYX 255 //the max as read
#define WIINUNCHUCK_CENTERJOYY 137 //the center as read
#define WIINUNCHUCK_INMINJOYY 0 //the min as read
#define WIINUNCHUCK_INMAXJOYY 255 //the max as read
#else
#define WIINUNCHUCK_DEFAULTZEROJOYX 137 //the center as read
#define WIINUNCHUCK_DEFAULTZEROJOYY 136 //the center as read
#endif

//decodebyte is necessary with certain initializations
#define WIINUNCHUCK_DECODEBYTE 0 //enable or disable the decode byte function
#if WIINUNCHUCK_DECODEBYTE == 1
#define wiinunchuck_decode(x) (x ^ 0x17) + 0x17;
#endif


/*
 * get joypad X
 */
int wiinunchuck_getjoyX(struct wiinunchuck *self) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1

	return (int)self->joyX;

	#else

	#if WIINUNCHUCK_JOYCENTERB == 1
	int joyX = (int)self->joyX;
	if(joyX <= WIINUNCHUCK_CENTERJOYX)
		joyX = (joyX - WIINUNCHUCK_INMINJOYX) * (127 - 0) / (WIINUNCHUCK_CENTERJOYX - WIINUNCHUCK_INMINJOYX) + 0;
	else
		joyX = (joyX - WIINUNCHUCK_CENTERJOYX) * (255 - 127) / (WIINUNCHUCK_INMAXJOYX - WIINUNCHUCK_CENTERJOYX) + 127 - 1;
	return joyX-127;
	#else
	return (int)self->joyX - WIINUNCHUCK_DEFAULTZEROJOYX;
	#endif

	#endif
}

/*
 * get joypad Y
 */
int wiinunchuck_getjoyY(struct wiinunchuck *self) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1

	return (int)self->joyY;

	#else

	#if WIINUNCHUCK_JOYCENTERB == 1
	int joyY = (int)self->joyY;
	if(joyY <= WIINUNCHUCK_CENTERJOYY)
		joyY = (joyY - WIINUNCHUCK_INMINJOYY) * (127 - 0) / (WIINUNCHUCK_CENTERJOYY - WIINUNCHUCK_INMINJOYY) + 0;
	else
		joyY = (joyY - WIINUNCHUCK_CENTERJOYY) * (255 - 127) / (WIINUNCHUCK_INMAXJOYY - WIINUNCHUCK_CENTERJOYY) + 127 - 1;
	return joyY-127;
	#else
	return (int)self->joyY - WIINUNCHUCK_DEFAULTZEROJOYY;
	#endif

	#endif
}

/*
 * get button Z
 */
uint8_t wiinunchuck_getbuttonZ(struct wiinunchuck *self) {
	#if WIINUNCHUCK_PULSEBUTTON == 1
	return (self->buttonZ && !self->lastbuttonZ);
	#else
	return self->buttonZ;
	#endif
}

/*
 * get button C
 */
uint8_t wiinunchuck_getbuttonC(struct wiinunchuck *self) {
	#if WIINUNCHUCK_PULSEBUTTON == 1
	return (self->buttonC && !self->lastbuttonC);
	#else
	return self->buttonC;
	#endif
}

/*
 * get angle X
 */
int wiinunchuck_getangleX(struct wiinunchuck *self) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1
	return self->angleX;
	#else
	return self->angleX - WIINUNCHUCK_ZEROANGLEX;
	#endif
}

/*
 * get angle Y
 */
int wiinunchuck_getangleY(struct wiinunchuck *self) {
	#if WIINUNCHUCK_GETNONCALIBRATED == 1
	return self->angleY;
	#else
	return self->angleY - WIINUNCHUCK_ZEROANGLEY;
	#endif
}

/*
 * get angle Z
 */
int wiinunchuck_getangleZ(struct wiinunchuck *self) {
	return self->angleZ - WIINUNCHUCK_ZEROANGLEZ;
}

void wiinunchuck_getpitchroll(struct wiinunchuck *self, double ax, double ay, double az, double *pitch, double *roll) {
	(void)(self); 
	double magnitude = sqrt(((uint32_t)ax * (uint32_t)ax) + ((uint32_t)ay * (uint32_t)ay) + ((uint32_t)az * (uint32_t)az));
	ax = ax / magnitude;
	ay = ay / magnitude;
	az = az / magnitude;
	*pitch = atan2(ax, sqrt(pow(ay,2) + pow(az, 2)));
	*roll = atan2(ay, az);
}

#if WIINUNCHUCK_ANGLEFILTER == 1
/*
 * avarage filter over an array
 */
static int wiinunchuck_avaragefilter(struct wiinunchuck *self, int input, int  *wiinunchuck_avarageangle) {
	(void)(self); 
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
void wiinunchuck_update(struct wiinunchuck *self) {
	uint8_t buff[WIINUNCHUCK_READBYTES];
	memset(buff, 0, sizeof(buff));
	
	//request data
	i2c_start_write(self->i2c, WIINUNCHUCK_ADDR, buff, 1);
	i2c_start_read(self->i2c, WIINUNCHUCK_ADDR, buff, WIINUNCHUCK_READBYTES);
	i2c_stop(self->i2c);
	
	/*
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
*/
	//decodebyte
	#if WIINUNCHUCK_DECODEBYTE == 1
	for(i=0; i<WIINUNCHUCK_READBYTES; i++) {
		buff[i] = wiinunchuck_decode(self, buff[i]);
	}
	#endif

	//record old records
	#if WIINUNCHUCK_PULSEBUTTON == 1
	self->lastbuttonZ = self->buttonZ;
	self->lastbuttonC = self->buttonC;
	#endif

	//get joypad
	self->joyX = buff[0];
	self->joyY = buff[1];

	//get button
	self->buttonZ = !(buff[5] & 0b00000001);
	self->buttonC = !((buff[5] & 0b00000010) >> 1);

	//get angle
	self->angleX = (buff[2] << 2) + ((buff[5] & (0b00000011 << (1*2)) >> (1*2)));
	self->angleY = (buff[3] << 2) + ((buff[5] & (0b00000011 << (2*2)) >> (2*2)));
	self->angleZ = (buff[4] << 2) + ((buff[5] & (0b00000011 << (3*2)) >> (3*2)));

	//filter angle
	#if WIINUNCHUCK_ANGLEFILTER == 1
	self->angleX = wiinunchuck_avaragefilter(self, self->angleX, self->avarageangleX);
	self->angleY = wiinunchuck_avaragefilter(self, self->angleY, self->avarageangleY);
	self->angleZ = wiinunchuck_avaragefilter(self, self->angleZ, self->avarageangleZ);
	#endif
}


/*
 * init wiinunchuck
 */ 
void wiinunchuck_init(struct wiinunchuck *self, i2c_dev_t i2c) {
	memset(self, 0, sizeof(struct wiinunchuck));
	
	self->i2c = i2c;
	uint8_t buf[2] = {0, 0}; 

	//standard init: 0x40 -> 0x00
	//alternative init: 0xF0 -> 0x55 followed by 0xFB -> 0x00, lets us use 3rd party nunchucks
	//no longer need to decode bytes in _nunchuk_decode_byte
	buf[0] = 0xF0; buf[1] = 0x55;
	i2c_start_write(self->i2c, WIINUNCHUCK_ADDR, buf, 2);
	i2c_stop(self->i2c);
	buf[0] = 0xFB; buf[1] = 0x00;
	i2c_start_write(self->i2c, WIINUNCHUCK_ADDR, buf, 2);
	i2c_stop(self->i2c);
	//update
	wiinunchuck_update(self);
}




