/**
	Fast ADC implementation using macros

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

#ifndef ADC_H
#define ADC_H

//setup values
#define ADC_REF 1 //adc reference voltage (see adc_init function for reference)
#define ADC_PRESCALER 128 //adc prescaler (2, 4, 8, 16, 32, 64, 128)
#define ADC_JUSTIFY 'R' //adc justify ('L' or 'R')
#define ADC_TRIGGERON 0 //adc trigger (1 on - 0 off)
#define ADC_REFRES 1024 //reference resolution used for conversions

//bandgap reference voltage * 1000, refers to datasheet
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
#define ADC_BANDGAPVOLTAGE 1100L
#elif defined (__AVR_ATmega8__)
#define ADC_BANDGAPVOLTAGE 1300L
#endif

#define ADC_REF_AREF (0)
#define ADC_REF_AVCC_CAP_AREF (_BV(REFS0))
#define ADC_REF_INTERNAL_2_56V (_BV(REFS1) | _BV(REFS0))
#define ADC_REF_BITS ADC_REF_INTERNAL_2_56V

#define adc0_set_vref(adcref) (ADMUX = (ADMUX & ~ADC_REF_BITS) | (adcref & ADC_REF_BITS))

#define ADC_CLOCK_DIV2 (_BV(ADPS0))
#define ADC_CLOCK_DIV4 (_BV(ADPS1))
#define ADC_CLOCK_DIV8 (_BV(ADPS1) | _BV(ADPS0))
#define ADC_CLOCK_DIV16 (_BV(ADPS2))
#define ADC_CLOCK_DIV32 (_BV(ADPS2) | _BV(ADPS0))
#define ADC_CLOCK_DIV64 (_BV(ADPS2) | _BV(ADPS1))
#define ADC_CLOCK_DIV128 (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0))
#define ADC_CLOCK_BITS ADC_CLOCK_DIV128

#define adc0_set_prescaler(adc_clock) (\
	ADCSRA = (ADCSRA &  ~ADC_CLOCK_BITS) | ((adc_clock) & ADC_CLOCK_BITS)\
)

#define ADC_ALIGN_LEFT (_BV(ADLAR))
#define ADC_ALIGN_RIGHT (0)

#define adc0_set_alignment(adc_align) (ADMUX = (ADMUX & ~(_BV(ADLAR))) | ((adc_align) & _BV(ADLAR))

#define adc0_enable() (ADCSRA |= _BV(ADEN))
#define adc0_disable() (ADCSRA &= ~_BV(ADEN))
#define adc0_init_default() (\
	adc0_set_vref(ADC_REF_AVCC_CAP_AREF),\
	adc0_set_prescaler(ADC_CLOCK_DIV128),\
	adc0_set_alignment(ADC_ALIGN_LEFT), \
	adc0_enable()\
)

#define adc0_interrupt_on() (ADCSRA |= _BV(ADIE))
#define adc0_interrupt_off() (ADCSRA &= ~_BV(ADIE))

#define ADC_CH0 (0)
#define ADC_CH1 (1)
#define ADC_CH2 (2)
#define ADC_CH3 (3)
#define ADC_CH4 (4)
#define ADC_CH5 (5)
#define ADC_CH6 (6)
#define ADC_CH7 (7)
#define ADC_CH_VREF (14)
#define ADC_CH_GND (15)
#define ADC_CH_BITS (0x0f)

#define adc0_set_channel(adc_chan) ({\
	adc0_disable(); \
	ADMUX = (ADMUX & ~ADC_CH_BITS) | ((adc_chan) & ADC_CH_BITS); \
	adc0_enable(); \
})

#define adc0_get_channel() (ADMUX & 0x07)

#define adc0_start_conversion() (ADCSRA |= (1 << ADSC))
#define adc0_conversion_in_progress() (ADCSRA & _BV(ADSC))
#define adc0_wait_for_completed_conversion() ({while(adc0_conversion_in_progress());})

#define adc0_read() (uint16_t)(\
	adc0_wait_for_completed_conversion(), \
ADC)

#define adc0_read_immediate(chan) (\
	adc0_wait_for_completed_conversion(), \
	adc0_interrupt_off(),\
	adc0_set_channel(chan), \
	adc0_start_conversion(), \
	(_adc_mode == ADC_MODE_AUTOMATIC)\
		?(adc0_interrupt_on(), adc0_start_conversion())\
		:(0)\
, adc0_read())

#define adc0_read_immediate_ref(chan, ref) (\
	adc0_wait_for_completed_conversion(), \
	adc0_interrupt_off(),\
	adc0_set_channel(chan), \
	adc0_set_vref(ref), \
	adc0_start_conversion(), \
	adc0_set_mode(_adc_mode)\
, adc0_read())

#define ADC_MODE_MANUAL (0)
#define ADC_MODE_AUTOMATIC (1)
extern uint8_t _adc_mode;

#if defined(CONFIG_ADC_MODE_AUTOMATIC)

	extern uint16_t _adc_values[8];

	#define adc0_read_cached(chan) (_adc_values[(chan) & 0x07])
#endif

#define adc0_set_mode(adc_mode) (\
	_adc_mode = adc_mode,\
	(_adc_mode == ADC_MODE_AUTOMATIC)\
		?(adc0_interrupt_on(), adc0_start_conversion())\
		:(0) \
)

#endif
