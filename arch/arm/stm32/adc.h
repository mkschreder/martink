#pragma once

#define ADC_REF_CAP_AREF 0

#define adc0_read_immediate_ref(chan, ref) (0)

void adc_init(void); 
uint16_t adc_read(uint8_t channel); 
