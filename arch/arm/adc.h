#pragma once

// TODO: BIG todo

#define ADC_REF_AREF (0)
#define ADC_REF_AVCC_CAP_AREF (0)
#define ADC_REF_INTERNAL_2_56V (0)
#define ADC_REF_BITS ADC_REF_INTERNAL_2_56V

#define adc0_set_vref(adcref) (0)

#define adc0_set_prescaler(adc_clock) (0)

#define ADC_ALIGN_LEFT (0)
#define ADC_ALIGN_RIGHT (0)

#define adc0_set_alignment(adc_align) (0)

#define adc0_enable() (0)
#define adc0_disable() (0)
#define adc0_init_default() (0)

#define adc0_interrupt_on() (0)
#define adc0_interrupt_off() (0)

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

#define adc0_set_channel(adc_chan) (0)

#define adc0_get_channel() (0)

#define adc0_start_conversion() (0)
#define adc0_conversion_in_progress() (0)
#define adc0_wait_for_completed_conversion() (0)

#define adc0_read() (uint16_t)(0)

#define adc0_read_immediate(chan) (0)

#define adc0_read_immediate_ref(chan, ref) (0)

#define ADC_MODE_MANUAL (0)
#define ADC_MODE_AUTOMATIC (1)
