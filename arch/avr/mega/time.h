/*
	This file is part of martink kernel library

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


#ifdef __cplusplus
extern "C" {
#endif

#include <arch/time.h>
/*
#define time_static_delay_us(us) _delay_us(us)

#define time_before(unknown, known) ((timestamp_t)(unknown) - (timestamp_t)(known) < 0)
#define time_after(a,b) time_before(b, a)
#define timeout_from_now(us) (time_get_clock() + time_us_to_clock(us))
// timeout expired: can handle overflow of timer correctly
#define timeout_expired(timeout) (time_after(time_get_clock(), timeout))
#define time_between(a, b) (a - b)
 
void time_init(void);

// returns current number of clock cycles (will overflow!)
timestamp_t time_get_clock(void); 

// converts a value in microseconds to number of clock ticks
timestamp_t time_us_to_clock(timestamp_t us);
timestamp_t time_clock_to_us(timestamp_t clock); 
*/
/// *************************
/// HARDWARE TIMER0 FUNCTIONS
/// *************************

#define TIM0_MODE_NORMAL					(0)
#define TIM0_MODE_PCPWM_TOPMAX		(_BV(WGM00))
#define TIM0_MODE_CTC							(_BV(WGM01))
#define TIM0_MODE_FASTPWM_TOPMAX	(_BV(WGM01) | _BV(WGM00))
#define TIM0_MODE_PCPWM_TOPOC0A		(_BV(WGM02) | _BV(WGM00))
#define TIM0_MODE_FASTPWM_TOPOC0A	(_BV(WGM02) | _BV(WGM01) | _BV(WGM00))

#define TIM0_OUTA_MODE_NONE				(0)
#define TIM0_OUTA_MODE_TOGGLE			(_BV(COM0A0))
#define TIM0_OUTA_MODE_CLEAR			(_BV(COM0A1))
#define TIM0_OUTA_MODE_SET				(_BV(COM0A1) | _BV(COM0A0))

#define TIM0_OUTB_MODE_NONE				(0)
#define TIM0_OUTB_MODE_TOGGLE			(_BV(COM0B0))
#define TIM0_OUTB_MODE_CLEAR			(_BV(COM0B1))
#define TIM0_OUTB_MODE_SET				(_BV(COM0B1) | _BV(COM0B0))

// wgm02 is in tccr0b, but one bit to the right
#define timer0_mode(tim0_mode) (\
	TCCR0A = ((TCCR0A & ~(_BV(WGM01) | _BV(WGM00)))) | \
		((tim0_mode) & ~(_BV(WGM02))), \
	TCCR0B = (TCCR0B & ~_BV(WGM02)) | ((tim0_mode) & (_BV(WGM02)))\
)

#define timer0_outa_mode(tim0_a_mode) (\
	TCCR0A = (TCCR0A & ~(_BV(COM0A1) | _BV(COM0A0))) | (tim0_a_mode)\
)

#define timer0_outb_mode(tim0_b_mode) (\
	TCCR0A = (TCCR0A & ~(_BV(COM0B1) | _BV(COM0B0))) | (tim0_b_mode)\
)

#define timer0_force_output_compare_a() (TCCR0B |= _BV(FOC0A))
#define timer0_force_output_compare_b() (TCCR0B |= _BV(FOC0B))

#define TIM0_CLOCK_OFF 			(0)
#define TIM0_CLOCK_DIV1			(_BV(CS00))
#define TIM0_CLOCK_DIV8			(_BV(CS01))
#define TIM0_CLOCK_DIV64		(_BV(CS01) | _BV(CS00))
#define TIM0_CLOCK_DIV256 	(_BV(CS02))
#define TIM0_CLOCK_DIV1024	(_BV(CS02) | _BV(CS00))
#define TIM0_CLOCK_EXT_FALL (_BV(CS02) | _BV(CS01))
#define TIM0_CLOCK_EXT_RISE (_BV(CS02) | _BV(CS01) | _BV(CS00))

#define timer0_set_clock(tim0_clock) (\
	TCCR0B = (TCCR0B & ~(_BV(CS02) | _BV(CS01) | _BV(CS00))) | tim0_clock\
)

#define timer0_set_channel_a(u8_val) (OCR0A = u8_val)
#define timer0_set_channel_b(u8_val) (OCR0B = u8_val)

#define timer0_interrupt_compa_on() (TIMSK0 |= _BV(OCIE0A))
#define timer0_interrupt_compa_off() (TIMSK0 &= ~_BV(OCIE0A))
#define timer0_interrupt_compb_on() (TIMSK0 |= _BV(OCIE0B))
#define timer0_interrupt_compb_off() (TIMSK0 &= ~_BV(OCIE0B))
#define timer0_interrupt_overflow_on() (TIMSK0 |= _BV(TOIE0))
#define timer0_interrupt_overflow_off() (TIMSK0 &= ~_BV(TOIE0))

/// *************************
/// HARDWARE TIMER1 FUNCTIONS
/// *************************

#define TIM1_MODE_NORMAL					(0)
#define TIM1_MODE_PCPWM8_TOPMAX		(_BV(WGM10))
#define TIM1_MODE_PCPWM9_TOPMAX		(_BV(WGM11))
#define TIM1_MODE_PCPWM10_TOPMAX	(_BV(WGM11) | _BV(WGM10))
#define TIM1_MODE_CTC_TOPOC1A				(_BV(WGM12))
#define TIM1_MODE_FASTPWM8_TOPMAX		(_BV(WGM12) | _BV(WGM10))
#define TIM1_MODE_FASTPWM9_TOPMAX		(_BV(WGM12) | _BV(WGM11))
#define TIM1_MODE_FASTPWM10_TOPMAX	(_BV(WGM12) | _BV(WGM11) | _BV(WGM10))
#define TIM1_MODE_PCFCPWM_TOPICR1		(_BV(WGM13))
#define TIM1_MODE_PCFCPWM_TOPOC1A		(_BV(WGM13) | _BV(WGM10))
#define TIM1_MODE_PCPWM_TOPICR1			(_BV(WGM13) | _BV(WGM11))
#define TIM1_MODE_PCPWM_TOPOC1A			(_BV(WGM13) | _BV(WGM11) | _BV(WGM10))
#define TIM1_MODE_CTC_TOPICR				(_BV(WGM13) | _BV(WGM12))
#define TIM1_MODE_FASTPWM_TOPICR		(_BV(WGM13) | _BV(WGM12) | _BV(WGM11))
#define TIM1_MODE_FASTPWM_TOPOC1A		(_BV(WGM13) | _BV(WGM12) | _BV(WGM11) | _BV(WGM10))

#define TIM1_OUTA_MODE_NONE				(0)
#define TIM1_OUTA_MODE_TOGGLE			(_BV(COM1A0))
#define TIM1_OUTA_MODE_CLEAR			(_BV(COM1A1))
#define TIM1_OUTA_MODE_SET				(_BV(COM1A1) | _BV(COM1A0))

#define TIM1_OUTB_MODE_NONE				(0)
#define TIM1_OUTB_MODE_TOGGLE			(_BV(COM1B0))
#define TIM1_OUTB_MODE_CLEAR			(_BV(COM1B1))
#define TIM1_OUTB_MODE_SET				(_BV(COM1B1) | _BV(COM1B0))

// wgm02 is in tccr0b, but one bit to the right
#define timer1_mode(tim1_mode) (\
	TCCR1A = (TCCR1A & ~(_BV(WGM11) | _BV(WGM10))) | \
		((tim1_mode) & ~(_BV(WGM13) | _BV(WGM12))), \
	TCCR1B = (TCCR1B & ~(_BV(WGM13) | _BV(WGM12))) | \
		((tim1_mode) & (_BV(WGM13) | _BV(WGM12)))\
)

#define timer1_outa_mode(tim1_a_mode) (\
	TCCR1A = (TCCR1A & ~(_BV(COM1A1) | _BV(COM1A0))) | (tim1_a_mode)\
)

#define timer1_outb_mode(tim0_b_mode) (\
	TCCR1A = (TCCR1A & ~(_BV(COM1B1) | _BV(COM1B0))) | (tim1_b_mode)\
)

#define timer1_force_output_compare_a() (TCCR1C |= _BV(FOC1A))
#define timer1_force_output_compare_b() (TCCR1C |= _BV(FOC1B))

#define TIM1_CLOCK_OFF 			(0)
#define TIM1_CLOCK_DIV1			(_BV(CS10))
#define TIM1_CLOCK_DIV8			(_BV(CS11))
#define TIM1_CLOCK_DIV64		(_BV(CS11) | _BV(CS10))
#define TIM1_CLOCK_DIV256 	(_BV(CS12))
#define TIM1_CLOCK_DIV1024	(_BV(CS12) | _BV(CS10))
#define TIM1_CLOCK_EXT_FALL (_BV(CS12) | _BV(CS11))
#define TIM1_CLOCK_EXT_RISE (_BV(CS12) | _BV(CS11) | _BV(CS10))

#define timer1_set_clock(tim1_clock) (\
	TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11) | _BV(CS10))) | tim1_clock\
)

#define timer1_set_channel_a(u16_val) (OCR1A = u16_val)
#define timer1_set_channel_b(u16_val) (OCR1B = u16_val)
#define timer1_set_input_capture_register(val) (ICR1 = val)

#define timer1_interrupt_compa_on() 		(TIMSK1 |= _BV(OCIE1A))
#define timer1_interrupt_compa_off() 		(TIMSK1 &= ~_BV(OCIE1A))
#define timer1_interrupt_compb_on() 		(TIMSK1 |= _BV(OCIE1B))
#define timer1_interrupt_compb_off() 		(TIMSK1 &= ~_BV(OCIE1B))
#define timer1_interrupt_input_capture_on() 		(TIMSK1 |= _BV(ICIE1))
#define timer1_interrupt_input_capture_off() 		(TIMSK1 &= ~_BV(ICIE1))
#define timer1_interrupt_overflow_on() 	(TIMSK1 |= _BV(TOIE1))
#define timer1_interrupt_overflow_off() (TIMSK1 &= ~_BV(TOIE1))


/// *************************
/// HARDWARE TIMER2 FUNCTIONS
/// *************************

#define TIM2_MODE_NORMAL					(0)
#define TIM2_MODE_PCPWM_TOPMAX		(_BV(WGM20))
#define TIM2_MODE_CTC							(_BV(WGM21))
#define TIM2_MODE_FASTPWM_TOPMAX	(_BV(WGM21) | _BV(WGM20))
#define TIM2_MODE_PCPWM_TOPOC2A		(_BV(WGM22) | _BV(WGM20))
#define TIM2_MODE_FASTPWM_TOPOC2A	(_BV(WGM22) | _BV(WGM21) | _BV(WGM20))

#define TIM2_OUTA_MODE_NONE				(0)
#define TIM2_OUTA_MODE_TOGGLE			(_BV(COM2A0))
#define TIM2_OUTA_MODE_CLEAR			(_BV(COM2A1))
#define TIM2_OUTA_MODE_SET				(_BV(COM2A1) | _BV(COM2A0))

#define TIM2_OUTB_MODE_NONE				(0)
#define TIM2_OUTB_MODE_TOGGLE			(_BV(COM2B0))
#define TIM2_OUTB_MODE_CLEAR			(_BV(COM2B1))
#define TIM2_OUTB_MODE_SET				(_BV(COM2B1) | _BV(COM2B0))

// wgm02 is in tccr0b, but one bit to the right
#define timer2_mode(tim2_mode) (\
	TCCR2A = (TCCR2A & ~(_BV(WGM21) | _BV(WGM20))) | \
		((tim2_mode) & ~(_BV(WGM22))), \
	TCCR2B = (TCCR2B & ~_BV(WGM22)) | ((tim2_mode) & (_BV(WGM22)))\
)

#define timer2_outa_mode(tim2_a_mode) (\
	TCCR2A = (TCCR2A & ~(_BV(COM2A1) | _BV(COM2A0))) | (tim2_a_mode)\
)

#define timer2_outb_mode(tim2_b_mode) (\
	TCCR2A = (TCCR2A & ~(_BV(COM2B1) | _BV(COM2B0))) | (tim2_b_mode)\
)

#define timer2_force_output_compare_a() (TCCR2B |= _BV(FOC2A))
#define timer2_force_output_compare_b() (TCCR2B |= _BV(FOC2B))

#define TIM2_CLOCK_OFF 			(0)
#define TIM2_CLOCK_DIV1			(_BV(CS00))
#define TIM2_CLOCK_DIV8			(_BV(CS01))
#define TIM2_CLOCK_DIV32		(_BV(CS01) | _BV(CS00))
#define TIM2_CLOCK_DIV64 	(_BV(CS02))
#define TIM2_CLOCK_DIV128	(_BV(CS02) | _BV(CS00))
#define TIM2_CLOCK_DIV256 (_BV(CS02) | _BV(CS01))
#define TIM2_CLOCK_DIV1024 (_BV(CS02) | _BV(CS01) | _BV(CS00))

#define timer2_set_clock(tim2_clock) (\
	TCCR2B = (TCCR2B & ~(_BV(CS22) | _BV(CS21) | _BV(CS20))) | tim2_clock\
)

#define timer2_set_channel_a(u8_val) (OCR2A = u8_val)
#define timer2_set_channel_b(u8_val) (OCR2B = u8_val)

#define timer2_interrupt_compa_on() 		(TIMSK2 |= _BV(OCIE2A))
#define timer2_interrupt_compa_off() 		(TIMSK2 &= ~_BV(OCIE2A))
#define timer2_interrupt_compb_on() 		(TIMSK2 |= _BV(OCIE2B))
#define timer2_interrupt_compb_off() 		(TIMSK2 &= ~_BV(OCIE2B))
#define timer2_interrupt_overflow_on() 	(TIMSK2 |= _BV(TOIE2))
#define timer2_interrupt_overflow_off() (TIMSK2 &= ~_BV(TOIE2))

/// ****************************
/// CPU TIME STAMP COUNTER SETUP
/// ****************************

#include "../../types.h"


#if defined(CONFIG_TIMESTAMP_COUNTER)

	extern volatile timestamp_t _tsc_ovf;

	#define TSC_PRESCALER TIM1_CLOCK_DIV8
	#define TSC_TICKS_PER_US 2 //((timestamp_t)(F_CPU / 8L))
	
	#define tsc_init(void) {}
#if 0
	#define tsc_init(void) (\
		timer1_mode(TIM1_MODE_NORMAL),\
		timer1_set_clock(TSC_PRESCALER),\
		timer1_interrupt_overflow_on()\
	)
#endif

	timestamp_t tsc_read(void);

	static inline timestamp_t tsc_us_to_ticks(timestamp_t us) {
		return ((timestamp_t)(TSC_TICKS_PER_US * ((timestamp_t)us)));
	}

	static inline timestamp_t tsc_ticks_to_us(timestamp_t ticks){
		return (((timestamp_t)ticks) / TSC_TICKS_PER_US);
	}

	#define tsc_reset(void) (\
		_tsc_ovf = 0,\
	)
	//TCNT1 = 0 
#endif

#define udelay(us) { long delay = us; while(delay--) _delay_us(1); }

//#define static_delay_us(us) _delay_us(us)

#ifdef __cplusplus
}
#endif
