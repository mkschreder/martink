/*
imu10dof01 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

A 10DOF IMU based on chip
  + ADXL345
  + L3G4200D
  + HML5883L
  + BMP085

References:
  - Mahony complementary filter for attitude estimation
    http://www.x-io.co.uk
  - Alec Myers MyAHRS (gyro offset calibration)
    https://github.com/alecmyers/MyAHRS
*/


#ifndef IMU10DOF01_H_
#define IMU10DOF01_H_

//setup timer0 overflow event and define madgwickAHRSsampleFreq equal to timer0 frequency
//timerfreq = (FCPU / prescaler) / timerscale
//     timerscale 8-bit = 256
// es. 61 = (16000000 / 1024) / 256
//#define TIMER0_PRESCALER (1<<CS02)|(1<<CS00)

#define IMU10DOF01_BAROMETERENABLED 1 //enable or disable barometer

#define IMU10DOF01_GYROOFFSETRECALIBRATION 1 //enable or disable gyro offset recalibration

#if IMU10DOF01_BAROMETERENABLED == 1
//define how often update pressure
//pressure update ms = (1/timerfreq)*MU10DOF01_PRESSUREUPDATECOUNT
//set this at 1ms more or less
#define IMU10DOF01_PRESSUREUPDATECOUNT 61
#endif

#define imu10dof01_sampleFreq 61.0f // sample frequency in Hz
#define imu10dof01_twoKpDef (2.0f * 0.7f) // 2 * proportional gain
#define imu10dof01_twoKiDef (2.0f * 0.1f) // 2 * integral gain

//functions
extern void imu10dof01_init(void);
extern void imu10dof01_attitudeenabled(void);
extern void imu10dof01_attituddisabled(void);
extern void imu10dof01_getQuaternion(double *qw, double *qx, double *qy, double *qz) ;
extern void imu10dof01_getRollPitchYaw(double *roll, double *pitch, double *yaw);
#if IMU10DOF01_BAROMETERENABLED == 1
extern void imu10dof01_getPressure(double *pressure);
#endif

#endif
