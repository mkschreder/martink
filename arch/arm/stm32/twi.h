#pragma once 

int8_t twi_init(uint8_t dev_id); 
void twi_deinit(uint8_t dev_id); 
int8_t twi_start_write(uint8_t dev_id, uint8_t addr, const uint8_t *data, uint8_t sz); 
int8_t twi_start_read(uint8_t dev_id, uint8_t addr, uint8_t *data, uint8_t sz); 
int8_t twi_stop(uint8_t dev_id); 
//void twi_wait(uint8_t dev_id, uint8_t addr); 
uint8_t twi_busy(uint8_t dev_id); 
uint8_t twi_aquire(uint8_t dev_id); 
void twi_release(uint8_t dev_id); 
