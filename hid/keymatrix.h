#pragma once 

#include <kernel/cbuf.h>

struct keymatrix {
	struct cbuf keypresses; 
}; 

void keymatrix_init(struct keymatrix *self); 
void keymatrix_add_col_pin(struct keymatrix *self, gpio_pin_t pin); 
void keymatrix_add_row_pin(struct keymatrix *self, gpio_pin_t pin); 
