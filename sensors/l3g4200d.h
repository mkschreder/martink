/*
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

	Special thanks to:
	* Davide Gironi, original implementation
*/

#ifndef L3G4200D_H_
#define L3G4200D_H_

#ifdef __cplusplus
"C" {
#endif

//definitions
#define L3G4200D_ADDR (0x69<<1) //device address

struct l3g4200d {
	i2c_dev_t i2c;
	uint8_t addr;
	int8_t temperatureref;
	float offsetx, offsety, offsetz; 
};

//functions
void l3g4200d_init(struct l3g4200d *self, i2c_dev_t i2c, uint8_t addr);
void l3g4200d_setoffset(struct l3g4200d *self, float offsetx, float offsety, float offsetz);
void l3g4200d_read_raw(struct l3g4200d *self, int16_t *gxraw, int16_t *gyraw, int16_t *gzraw);
void l3g4200d_read_converted(struct l3g4200d *self, float* gx, float* gy, float* gz);
void l3g4200d_settemperatureref(struct l3g4200d *self);
int8_t l3g4200d_gettemperaturediff(struct l3g4200d *self);

#ifdef __cplusplus
}
#endif

#endif
