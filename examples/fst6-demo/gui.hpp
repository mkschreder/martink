#pragma once

struct gui_channel {
	uint32_t value; 
	uint32_t min; 
	uint32_t max; 
}; 

struct gui_switch {
	uint8_t on; 
};

struct gui_model {
	struct gui_channel ch[6]; 
	struct gui_switch sw[4]; 
	uint8_t calibrated; 
}; 

void gui_init(fbuf_dev_t fbuf); 
struct gui_model *gui_get_model(void); 
