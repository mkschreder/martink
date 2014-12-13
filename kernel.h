/**
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

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#pragma once 

#include "include/autoconf.h"
#include <inttypes.h>

#include <arch/soc.h>
#include <boards/board.h>
#include "util.h"

//#include "accel_docalibration.h"


#ifdef CONFIG_SSD1306
#include "disp/ssd1306.h"
#endif

#ifdef CONFIG_ACS712
#include "sensors/acs712.h"
#endif

#ifdef CONFIG_ADXL345
#include "sensors/adxl345.h"
#endif

#ifdef CONFIG_ATM1001
#include "sensors/amt1001.h"
#endif

//#include "audioget.h"
#ifdef CONFIG_AVRBUS
#include "avrbus.h"
#endif

#ifdef CONFIG_BH1750
#include "sensors/bh1750.h"
#endif

//#include "bldcsensored.h"
//#include "bldcsensorless.h"

#ifdef CONFIG_BMP085
#include "sensors/bmp085.h"
#endif

//#include "dcmotor.h"
//#include "dcmotorpwm.h"
//#include "dcmotorpwmsoft.h"

#ifdef CONFIG_DHT11
#include "sensors/dht.h"
#endif

#ifdef CONFIG_DS18B20
#include "sensors/ds18b20.h"
#endif

#ifdef CONFIG_ENC28J60
#include "net/enc28j60.h"
#endif

#ifdef CONFIG_FFT
#include "fftradix4.h"
#endif

#ifdef CONFIG_FS300A
#include "sensors/fs300a.h"
#endif

//#include "gyro_docalibration.h"
#ifdef CONFIG_HCSR04
#include "sensors/hcsr04.h"
#endif

#ifdef CONFIG_HMC5883L
#include "sensors/hmc5883l.h"
#endif

//#include "i2csoft.h"
//#include "i2csw.h"
//#include "i2csw_slave.h"

#ifdef CONFIG_ILI9340
#include "disp/ili9340.h"
#endif

#ifdef CONFIG_IMU10DOF01
#include "sensors/imu10dof01.h"
#endif

#ifdef CONFIG_L3G4200D
#include "sensors/l3g4200d.h"
#endif

#ifdef CONFIG_L74HC165
#include "io/l74hc165.h"
#endif

#ifdef CONFIG_L74HC4051
#include "io/l74hc4051.h"
#endif

#ifdef CONFIG_L74HC595
#include "io/l74hc595.h"
#endif

//#include "lcdpcf8574.h"
//#include "ldr.h"
//#include "ledmatrix88.h"
//#include "magn_docalibration.h"
#ifdef CONFIG_MFRC522
#include "rfid/mfrc522.h"
#endif

#ifdef CONFIG_MMA7455
#include "sensors/mma7455.h"
#endif

#ifdef CONFIG_MPU6050
#include "sensors/mpu6050.h"
#include "sensors/mpu6050registers.h"
#endif

#ifdef CONFIG_NRF24L01
#include "radio/nrf24l01.h"
#include "radio/nrf24l01registers.h"
#endif

//#include "ntctemp.h"
#ifdef CONFIG_PCF8574
#include "io/pcf8574.h"
#endif

//#include "pwm.h"
//#include "pwmcd4017.h"
//#include "pwmcd4017servo.h"
#ifdef CONFIG_RFNET
#include "net/rfnet.h"
#endif

//#include "sevseg.h"
//#include "softi2c.h"

#ifdef CONFIG_SSD1306
#include "disp/ssd1306.h"
#include "disp/ssd1306_priv.h"
#endif

//#include "stepper02.h"
//#include "stepper04multi.h"
#if defined(CONFIG_TCPIP) | defined(CONFIG_TCP) | defined(CONFIG_UDP)
#include "net/tcpip.h"
#endif

#ifdef CONFIG_VT100
#include "tty/vt100.h"
#endif

#ifdef CONFIG_HAVE_UART
	#define kprintf(a, ...) PFCALL(CONFIG_UART0_NAME, printf, a, ##__VA_ARGS__) 
	
	#ifdef CONFIG_DEBUG
		#define kdebug(a, args...) PFCALL(CONFIG_UART0_NAME, printf, a, args) 
	#else
		#define kdebug(a, ...) {}
	#endif
#else 
	#define kprintf(a, ...) {}
	#define kdebug(a, ...) {}
#endif

#define DECLARE_MAIN_MODULE() const struct main_module kmain_module = 
extern const struct main_module kmain_module; 

#ifdef __cplusplus
__extension__ typedef int __guard __attribute__((mode (__DI__)));

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *);
extern "C" void __cxa_pure_virtual(void);
#else 
	typedef int __guard; 
#endif

//#include "wiinunchuck.h"
