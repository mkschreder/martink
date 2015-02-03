/*
	This file is part of martink project. 

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Github: https://github.com/mkschreder

	Contributors:
	* Davide Gironi - developing original driver
	* Martin K. Schröder - maintenance since Oct 2014
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
