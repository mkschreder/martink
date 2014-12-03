/*
audioget 0x02

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef AUDIOGET_H
#define AUDIOGET_H

//adc input channel
#define AUDIOGET_INPUTCHANNEL 3

// BIAS setup ----------------
//enable or disable dynamic bias
//with dynamic bias the zero raw bias is estimated using max and min raw value read
#define AUDIOGET_DYNAMICBIAS 1
//zero value, raw value read when there is no sound
#define AUDIOGET_BIASZERORAW 512

// RMS setup ----------------
//define the smooth factor for the getval value (from 0:more filter to 64:less filter)
#define AUDIOGET_SMOOTHFILTERVAL 15

// INPUT setup ----------------
//get adc value using timer, sample values
#define AUDIOGET_USETIMER 0
//sampler timer
#if AUDIOGET_USETIMER == 1
//freq = FCPU/1+(prescaler*ICR1)
//ICR1 = (FCPU/freq - 1)/prescaler
//note: ICR1 can not be small, because we have some commands to execute during the interrupt routine
//es. 10Khz @ 8Mhz, ICR1 100, prescaler 8
#define TIMER1_ICR1 100
#define TIMER1_PRESCALER (1<<CS11)

#define AUDIOGET_ADCSAMPLES 64 //must be equal to AUDIOGET_AVARAGESAMPLES, AUDIOGET_RMSSAMPLES or FFT_SIZE
#endif

// TYPE AVARAGE setup ----------------
//avarage number of samples to take for measure
#define AUDIOGET_AVARAGESAMPLES 64
//define the correction value for rms radix calculation
#define AUDIOGET_AVARAGECORRECTION 1

//functions
extern void audioget_init(void);
extern int16_t audioget_getrmsval(void);
extern int16_t audioget_getspl(float voltnow, float voltref, float dbref);

#endif
