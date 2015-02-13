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
#ifndef HMC5883L_H_
#define HMC5883L_H_

#ifdef __cplusplus
extern "C" {
#endif

//definitions
#define HMC5883L_ADDR (0x1E<<1) //device address

struct hmc5883l {
	i2c_dev_t i2c;
	uint8_t addr;
	float scale; 
};

//functions
void hmc5883l_init(struct hmc5883l *self, i2c_dev_t i2c, uint8_t addr);
void hmc5883l_readRawMag(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw);
void hmc5883l_read_adjusted(struct hmc5883l *self, float *mx, float *my, float *mz);
void hmc5883l_convertMag(struct hmc5883l *self, 
	int16_t mxraw, int16_t myraw, int16_t mzraw, 
	float *mx, float *my, float *mz); 
uint32_t hmc5883l_read_id(struct hmc5883l *self); 
#ifdef __cplusplus
}
#endif

#endif
