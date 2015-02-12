#pragma once 

void twi0_init_default(void); 
void twi0_start_write(uint8_t addr, const uint8_t *data, uint8_t sz); 
void twi0_start_read(uint8_t addr, uint8_t *data, uint8_t sz); 
int16_t twi0_stop(void); 
#define twi0_busy() (0)
void twi0_wait(uint8_t addr); 
