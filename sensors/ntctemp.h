/*
ntctemp 0x02

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  + Guillem Planissi NTC tutorial on avrfreaks.net
    http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=61128
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
