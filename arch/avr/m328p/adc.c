/*
ADC Library 0x05

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#if defined (__AVR_ATtiny13A__)
#elif defined (__AVR_ATmega8__)
#elif defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__)
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
#else
	#error "no definitions available for this AVR"
#endif

#include "adc.h"

//complete this if trigger is on
//call adc_setchannel()
//call sei()
//ISR(ADC_vect) 
//{
//}
/*
void adc_setchannel(uint8_t channel)
{
	ADCSRA &= ~(1 << ADEN);
	ADMUX = (ADMUX & 0xf8) | (channel & 0x07); //set channel
	ADCSRA |= (1 << ADEN);
}

uint16_t adc_readsel(void)
{
	ADCSRA |= (1 << ADSC); // Start conversion
	while(ADCSRA & _BV(ADSC)); 
	//while( !(ADCSRA & (1<<ADIF)) ); // Wait for conversion to complete
	uint16_t adc = ADC;
	//ADCSRA |= (1 << ADIF); // Clear ADIF by writing one to it
	return(adc);
}

uint16_t adc_read(uint8_t channel)
{
	adc_setchannel(channel);
	return adc_readsel();
}

void adc_init(void) 
{
	// Set ADC reference
	#if defined (__AVR_ATtiny13A__)
		#if ADC_REF == 0
		ADMUX |= (0 << REFS0); // VCC used as analog reference
		#elif ADC_REF == 1
		ADMUX |= (1 << REFS0); // Internal Voltage Reference
		#endif
	#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
		#if ADC_REF == 0
		ADMUX |= (0 << REFS1) | (0 << REFS0); // AREF, Internal Vref turned off
		#elif ADC_REF == 1
		ADMUX |= (0 << REFS1) | (1 << REFS0); // AVCC with external capacitor at AREF pin
		#elif ADC_REF == 3
		ADMUX |= (1 << REFS1) | (1 << REFS0); // Internal 2.56V Voltage Reference with external cap at AREF
		#endif
	#else 
		#error "No processor type defined!"
	#endif
	
	// Set ADC prescaler
	#if ADC_PRESCALER == 2
	ADCSRA |= (0 << ADPS2) | (0 << ADPS1) | (1 << ADPS0); // Prescaler 2
	#elif ADC_PRESCALER == 4
	ADCSRA |= (0 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // Prescaler 4
	#elif ADC_PRESCALER == 8
	ADCSRA |= (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 8
	#elif ADC_PRESCALER == 16
	ADCSRA |= (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0); // Prescaler 16
	#elif ADC_PRESCALER == 32
	ADCSRA |= (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0); // Prescaler 32
	#elif ADC_PRESCALER == 64
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // Prescaler 64
	#elif ADC_PRESCALER == 128
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
	#endif
	
	// Set ADC justify
	#if ADC_JUSTIFY == 'L'
	ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading
	#elif ADC_JUSTIFY == 'R'
	ADMUX |= (0 << ADLAR); // Right adjust
	#endif
	
	// Set ADC trigger and mode
	#if ADC_TRIGGERON == 1
		#if defined (__AVR_ATtiny13A__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
		ADCSRB |= (0 << ADTS2) | (0 << ADPS1) | (0 << ADPS0); // Free Running mode
		ADCSRA |= (1 << ADATE); // Enable ADC Interrupt
		#elif defined (__AVR_ATmega8__)
		ADCSRA |= (1 << ADFR); // Set ADC to Free-Running Mode
		#endif
		ADCSRA |= (1 << ADIE); // Enable ADC Interrupt
	#else 
		ADCSRB |= (0 << ADTS2) | (0 << ADPS1) | (0 << ADPS0); // Free Running mode
	#endif
	
	// Enable ADC
	ADCSRA |= (1 << ADEN);

	//DIDR0 = 0xff;
	
	#if	ADC_TRIGGERON == 1
	ADCSRA |= (1 << ADSC); // Start conversions
	#endif
}

double acd_getrealvref(void)
{
	double intvoltage = 0;
	#if defined (__AVR_ATmega8__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
	//set bandgap voltage channel, and read value
	adc_setchannel(14);
	_delay_us(250);
	uint16_t adc = adc_readsel();
	//calculate internal voltage
	intvoltage = ((ADC_BANDGAPVOLTAGE * ADC_REFRES) / adc) / 1000.0;
	#endif
	return intvoltage;
}

long adc_getresistence(uint16_t adcread, uint16_t adcbalanceresistor)
{
	if(adcread == 0)
		return 0;
	else
		return (long)((long)(ADC_REFRES*(long)adcbalanceresistor)/adcread-(long)adcbalanceresistor);
}

double adc_getvoltage(uint16_t adcread, double adcvref) {
	if(adcread == 0)
		return 0;
	else
		return (double)(adcread*adcvref/(double)ADC_REFRES);
}

#define ADC_EMAFILTERALPHA 30
unsigned int adc_emafilter(unsigned int newvalue, unsigned int value)
{
	//use exponential moving avarate Y=(1-alpha)*Y + alpha*Ynew, alpha between 1 and 0
	//in uM we use int math, so Y=(63-63alpha)*Y + 63alpha*Ynew  and  Y=Y/63 (Y=Y>>6)
	value = (64-ADC_EMAFILTERALPHA)*value+ADC_EMAFILTERALPHA*newvalue;
	value = (value>>6);
	return value;
}
*/
