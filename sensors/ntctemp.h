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

#ifndef NTCTEMP_H
#define NTCTEMP_H

#include <stdio.h>

//enable the B Model Equation
#define NTCTEMP_B 1 //1 to enable 0 to disable
//enable the SH Model Equation
#define NTCTEMP_SH 1 //1 to enable 0 to disable
//enable the lookup table Equation
#define NTCTEMP_LKP 0 //1 to enable 0 to disable

#if NTCTEMP_B == 1
extern float ntctemp_getB(long adcresistence, int beta, float adctref, int adcrref);
#endif

#if NTCTEMP_SH == 1
extern float ntctemp_getSH(long adcresistence, float A, float B, float C);
#endif

#if NTCTEMP_LKP == 1
#define NTCTEMP_LOOKUPRETERROR -32767
extern float ntctemp_getLookup(uint16_t adcvalue);
#endif

#endif
