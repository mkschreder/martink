#pragma once

struct gui_channel_settings {
	uint8_t id; 
	uint32_t min; 
	uint32_t max; 
	uint8_t reverse; 
	uint8_t rate; 
	uint8_t exponent; 
	uint8_t source; 
	int8_t offset; 
}; 

struct gui_switch {
	uint8_t on; 
};

struct gui_profile_settings {
	char name[8]; 
	uint8_t mode_id; 
}; 

struct gui_output_channel {
	uint32_t value; 
}; 

struct gui_data {
	struct gui_output_channel out[6]; 
	uint8_t sw[4]; 
	struct gui_profile_settings profile; // current profile
	struct gui_channel_settings channel; // current channel
}; 

void gui_init(fbuf_dev_t fbuf); 
struct gui_data *gui_get_data(void); 
