/*
audioget 0x02

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>  //include libm

#include "audioget.h"

#include "arch/m328p/adc.h"

#if AUDIOGET_USETIMER == 1

volatile int16_t audioget_adcsamples[AUDIOGET_ADCSAMPLES] = {0};
volatile uint8_t audioget_adccollected = 0;
/*
 * timer1 interrupt
 */
ISR(TIMER1_OVF_vect) {
	static uint16_t audioget_adcsamplescollected = 0;

	#if AUDIOGET_DYNAMICBIAS == 1
	static int16_t biasadc = 0;
	static int16_t biasadcmin = 1024;
	static int16_t biasadcmax = 0;
	#endif

	if(audioget_adccollected == 0) {
		if(audioget_adcsamplescollected < AUDIOGET_ADCSAMPLES) {

			int16_t adc_raw = adc_readsel();
			#if AUDIOGET_DYNAMICBIAS == 1
			if(adc_raw < biasadcmin)
				biasadcmin = adc_raw;
			if(adc_raw > biasadcmax)
				biasadcmax = adc_raw;
			biasadc = (biasadcmin + (biasadcmax-biasadcmin)/2);
			adc_raw = adc_raw - biasadc;
			#else
			adc_raw = adc_raw - AUDIOGET_BIASZERORAW;
			#endif

			audioget_adcsamples[audioget_adcsamplescollected] = adc_raw;
			audioget_adcsamplescollected++;
		} else {
			#if AUDIOGET_DYNAMICBIAS == 1
			biasadc = 0;
			biasadcmin = 1024;
			biasadcmax = 0;
			#endif

			audioget_adcsamplescollected = 0;
			audioget_adccollected = 1;
		}
	}
}
#endif


/*
 * init ports
 */
void audioget_init(void) {
	//init adc
	adc_setchannel(AUDIOGET_INPUTCHANNEL);
	adc_init();

	#if AUDIOGET_USETIMER == 1
	//init timer
	ICR1 = TIMER1_ICR1; //ICR1
	TCCR1A = (1 << WGM11); //fast pwm mode
	TCCR1B = (1 << WGM13) | (1 << WGM12);
	TCCR1B |= TIMER1_PRESCALER; //prescaler
	TIMSK |= (1<<TOIE1); //enable timer1
	#endif

	//enable interrupt
	sei();
}


/*
 * get rms value from audio signal
 * signal must be biased to ADVREF voltage / 2
 */
int16_t audioget_getrmsval(void) {
	static int16_t retval = 0;
	int16_t getval = 0;

	#if AUDIOGET_USETIMER == 1
	while(!audioget_adccollected);
	audioget_adccollected = 1;
	#endif

	#if AUDIOGET_USETIMER == 0
	#if AUDIOGET_DYNAMICBIAS == 1
	int16_t biasadc = 0;
	int16_t biasadcmin = 1024;
	int16_t biasadcmax = 0;
	#endif
	#endif


	//get value using avarage method
	//---------------------------------------
	int32_t rmssquaresum = 0;
	int16_t adc_raw = 0;
	#if AUDIOGET_USETIMER == 0
	adc_setchannel(AUDIOGET_INPUTCHANNEL);
	#endif

	//root mean square to measure value
	uint8_t i = 0;
	for(i=0; i<AUDIOGET_AVARAGESAMPLES; i++) {
		#if AUDIOGET_USETIMER == 1
		adc_raw = audioget_adcsamples[i];
		#else
		adc_raw = adc_readsel();
		#if AUDIOGET_DYNAMICBIAS == 1
		if(adc_raw < biasadcmin)
			biasadcmin = adc_raw;
		if(adc_raw > biasadcmax)
			biasadcmax = adc_raw;
		biasadc = (biasadcmin + (biasadcmax-biasadcmin)/2);
		adc_raw = adc_raw - biasadc;
		#else
		adc_raw = adc_raw - AUDIOGET_BIASZERORAW;
		#endif
		#endif

		rmssquaresum += ((int32_t)adc_raw * (int32_t)adc_raw);
	}
	//root
	getval = sqrt(rmssquaresum/AUDIOGET_AVARAGESAMPLES)-AUDIOGET_AVARAGECORRECTION;


	//smooth filter
	retval = ((int32_t)(64-AUDIOGET_SMOOTHFILTERVAL)*retval+(int32_t)AUDIOGET_SMOOTHFILTERVAL*getval)>>6;

	if(retval<0)
		retval = 0;

	#if AUDIOGET_USETIMER == 1
	audioget_adccollected = 0;
	#endif

	return retval;
}

/*
 * get spl value,
 * depending on actual voltage read, voltage at a db reference, the db reference
 */
int16_t audioget_getspl(float voltnow, float voltref, float dbref) {
	int16_t ret = (20 * log10((voltnow/voltref)+1)) +  dbref;
	return ret;
}

