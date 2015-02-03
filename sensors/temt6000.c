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

#include <stdio.h>

#include "temt6000.h"

/*
 * get lux from voltage
 */
double temt6000_getlux(double vol) {
	//suppose we read the voltage from the emitter of temt6000, with a 10k pulldown resistor connected to gnd
	//V[V] = I[A]*R[ohm] = I[uA] * 1[A]/1000000[µA] * 10000[ohm] = I[uA] * 1[A]/1[uA] * 0.01[ohm]
	//1[uA] = V[V] / 0.01[ohm] = 100 * V[V/ohm]
	//the curve from datasheet seems y=2x+0, so lux = 2 * I[uA] = 2 * 100 * V[V/ohm]
	return 2*(100*vol);
}


