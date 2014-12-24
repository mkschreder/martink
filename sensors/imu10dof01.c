/*
imu10dof01 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <math.h>  //include libm

#include <arch/soc.h>

#include "imu10dof01.h"

#include "adxl345.h"
#include "l3g4200d.h"
#include "hmc5883l.h"
#include "bmp085.h"

// TODO: make it use new driver functions

//path to i2c fleury lib
#include IMU10DOF01_I2CFLEURYPATH

volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
volatile float a0 = 0.0f;

#if IMU10DOF01_BAROMETERENABLED == 1
uint16_t pressureupdatecounter = 0;
#endif

volatile static uint8_t imu10dof01_isattitudeenabled = 0;

#if IMU10DOF01_GYROOFFSETRECALIBRATION == 1
/*
 * calibrate gyro offset if the chip is not moving
 */
#define IMU10DOF01_imu10dof01_calibrategyrooffsetsteps 100
void imu10dof01_calibrategyrooffset(void) {
	int16_t gxraw = 0;
	int16_t gyraw = 0;
	int16_t gzraw = 0;
	int32_t gxrawtot = 0;
	int32_t gyrawtot = 0;
	int32_t gzrawtot = 0;

	double axg = 0;
	double ayg = 0;
	double azg = 0;
	double axgtot = 0;
	double aygtot = 0;
	double azgtot = 0;
	double axgtots = 0;
	double aygtots = 0;
	double azgtots = 0;

	uint8_t i = 0;

	for(i=0; i<IMU10DOF01_imu10dof01_calibrategyrooffsetsteps; i++) {
		//get accellerometer
		adxl345_getdata(&axg, &ayg, &azg);
		//do accel sum
		axgtot += axg;
		aygtot += ayg;
		azgtot += azg;
		//do accel squared sum
		axgtots += axg*axg;
		aygtots += ayg*ayg;
		azgtots += azg*azg;

		//get gyroscope
		l3g4200d_getrawdata(&gxraw, &gyraw, &gzraw);
		//do gyro raw sum
		gxrawtot += gxraw;
		gyrawtot += gyraw;
		gzrawtot += gzraw;
	}

	//calculate accel variance
	double accelvariancex = (axgtots - (axgtot*axgtot / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps)) / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps;
	double accelvariancey = (aygtots - (aygtot*aygtot / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps)) / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps;
	double accelvariancez = (azgtots - (azgtot*azgtot / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps)) / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps;

	//calculate accl variance magnitude
	double accelvariancex_magnitude = sqrt(accelvariancex*accelvariancex + accelvariancey*accelvariancey + accelvariancez*accelvariancez);
	if (accelvariancex_magnitude > 5.0e-4) {
		//moved during calibration, mantain old gyro offset
	} else {
		//stable during calibration, write new gyro offset
		double gyrooffsetx = gxrawtot / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps;
		double gyrooffsety = gyrawtot / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps;
		double gyrooffsetz = gzrawtot / IMU10DOF01_imu10dof01_calibrategyrooffsetsteps;
		l3g4200d_setoffset(gyrooffsetx, gyrooffsety, gyrooffsetz);
		#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
		l3g4200d_settemperatureref();
		#endif
	}
}
#endif

/*
 * initialize dof components
 */
void imu10dof01_init(void) {
	//init accellerometer
	adxl345_init();
	//init gyroscope
	l3g4200d_init();
	//init magnetometer
	hmc5883l_init();

	#if IMU10DOF01_BAROMETERENABLED == 1
	//init barometer
	bmp085_init();
	//get barometer first value
	a0 = bmp085_getpressure();
	#endif

	#if IMU10DOF01_GYROOFFSETRECALIBRATION == 1
	//wait for sensors to start, then try gyro offset calibration
	_delay_ms(200);
	imu10dof01_calibrategyrooffset();
	#endif

	//init timer
	TCCR0B |= TIMER0_PRESCALER;
	TIMSK0 |=(1<<TOIE0); //enable timer
}

/*
 * enable attitude computation
 */
void imu10dof01_attitudeenabled(void) {
	imu10dof01_isattitudeenabled = 1;
}

/*
 * disable attitude computation
 */
void imu10dof01_attituddisabled(void) {
	imu10dof01_isattitudeenabled = 0;
}

/*
 * Mahony update function (for 9DOF)
 */
void imu10dof01_mahonyUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float norm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
	float hx, hy, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		norm = sqrt(ax * ax + ay * ay + az * az);
		ax /= norm;
		ay /= norm;
		az /= norm;

		//if magnetometer measurement invalid process only using accellerometer and gyroscope
		if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
			// Estimated direction of gravity and vector perpendicular to magnetic flux
			halfvx = q1 * q3 - q0 * q2;
			halfvy = q0 * q1 + q2 * q3;
			halfvz = q0 * q0 - 0.5f + q3 * q3;

			// Error is sum of cross product between estimated and measured direction of gravity
			halfex = (ay * halfvz - az * halfvy);
			halfey = (az * halfvx - ax * halfvz);
			halfez = (ax * halfvy - ay * halfvx);
		} else {
			// Normalise magnetometer measurement
			norm = sqrt(mx * mx + my * my + mz * mz);
			mx /= norm;
			my /= norm;
			mz /= norm;

			// Auxiliary variables to avoid repeated arithmetic
			q0q0 = q0 * q0;
			q0q1 = q0 * q1;
			q0q2 = q0 * q2;
			q0q3 = q0 * q3;
			q1q1 = q1 * q1;
			q1q2 = q1 * q2;
			q1q3 = q1 * q3;
			q2q2 = q2 * q2;
			q2q3 = q2 * q3;
			q3q3 = q3 * q3;

			// Reference direction of Earth's magnetic field
			hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
			hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
			bx = sqrt(hx * hx + hy * hy);
			bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

			// Estimated direction of gravity and magnetic field
			halfvx = q1q3 - q0q2;
			halfvy = q0q1 + q2q3;
			halfvz = q0q0 - 0.5f + q3q3;
			halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
			halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
			halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);

			// Error is sum of cross product between estimated direction and measured direction of field vectors
			halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
			halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
			halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);
		}

		// Compute and apply integral feedback if enabled
		if(imu10dof01_twoKiDef > 0.0f) {
			integralFBx += imu10dof01_twoKiDef * halfex * (1.0f / imu10dof01_sampleFreq); // integral error scaled by Ki
			integralFBy += imu10dof01_twoKiDef * halfey * (1.0f / imu10dof01_sampleFreq);
			integralFBz += imu10dof01_twoKiDef * halfez * (1.0f / imu10dof01_sampleFreq);
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		} else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += imu10dof01_twoKpDef * halfex;
		gy += imu10dof01_twoKpDef * halfey;
		gz += imu10dof01_twoKpDef * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / imu10dof01_sampleFreq)); // pre-multiply common factors
	gy *= (0.5f * (1.0f / imu10dof01_sampleFreq));
	gz *= (0.5f * (1.0f / imu10dof01_sampleFreq));
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx);

	// Normalise quaternion
	norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 /= norm;
	q1 /= norm;
	q2 /= norm;
	q3 /= norm;
}

/*
 * update quaternion
 */
void imu10dof01_updateQuaternion(void) {
	double axg = 0;
	double ayg = 0;
	double azg = 0;
	double gxrs = 0;
	double gyrs = 0;
	double gzrs = 0;
	double mxg = 0;
	double myg = 0;
	double mzg = 0;

	//get gyroscope
	l3g4200d_getdata(&gxrs, &gyrs, &gzrs);
    gxrs = gxrs*0.01745329; //degree to radians
    gyrs = gyrs*0.01745329; //degree to radians
    gzrs = gzrs*0.01745329; //degree to radians
	//get accellerometer
	adxl345_getdata(&axg, &ayg, &azg);
    //get magnetometer
	hmc5883l_getdata(&mxg, &myg, &mzg);

    //compute data
    imu10dof01_mahonyUpdate(gxrs, gyrs, gzrs, axg, ayg, azg, mxg, myg, mzg);
}

/*
 * update timer for attitude
 */
ISR(TIMER0_OVF_vect) {
	if(imu10dof01_isattitudeenabled) {
		//update quaternion
		imu10dof01_updateQuaternion();
		#if IMU10DOF01_BAROMETERENABLED == 1
		//update pressure
		pressureupdatecounter++;
		if(pressureupdatecounter == IMU10DOF01_PRESSUREUPDATECOUNT) {
			pressureupdatecounter = 0;
			a0 = bmp085_getpressure();
		}
		#endif
	}
}

/*
 * get quaternion
 */
void imu10dof01_getQuaternion(double *qw, double *qx, double *qy, double *qz) {
	*qw = q0;
	*qx = q1;
	*qy = q2;
	*qz = q3;
}
/*
 * get euler angles
 * aerospace sequence, to obtain sensor attitude:
 * 1. rotate around sensor Z plane by yaw
 * 2. rotate around sensor Y plane by pitch
 * 3. rotate around sensor X plane by roll
 */
void imu10dof01_getRollPitchYaw(double *roll, double *pitch, double *yaw) {
    *yaw = atan2(2*q1*q2 - 2*q0*q3, 2*q0*q0 + 2*q1*q1 - 1);
    *pitch = -asin(2*q1*q3 + 2*q0*q2);
    *roll = atan2(2*q2*q3 - 2*q0*q1, 2*q0*q0 + 2*q3*q3 - 1);
}

#if IMU10DOF01_BAROMETERENABLED == 1

void imu10dof01_getPressure(double *pressure) {
	*pressure = a0;
}
#endif


