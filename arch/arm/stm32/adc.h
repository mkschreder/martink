#pragma once

#define ADC_REF_CAP_AREF 0

#define adc0_read_immediate_ref(chan, ref) (0)

void adc_init(void); 
uint8_t adc_aquire(uint8_t chan); 
void adc_release(uint8_t chan);
uint8_t adc_busy(void); 
int8_t adc_start_read(uint8_t channel, uint16_t *reading); 
