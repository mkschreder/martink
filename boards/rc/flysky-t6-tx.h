#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	FST6_KEY_NONE, 
	FST6_KEY_CH1P = (1 << 0), 
	FST6_KEY_CH1M = (1 << 1),  
	FST6_KEY_CH2P = (1 << 2), 
	FST6_KEY_CH2M = (1 << 3), 
	FST6_KEY_CH3P = (1 << 4), 
	FST6_KEY_CH3M = (1 << 5), 
	FST6_KEY_CH4P = (1 << 6), 
	FST6_KEY_CH4M = (1 << 7),
	FST6_KEY_OK = 	(1 << 8), 
	FST6_KEY_CANCEL = (1 << 9),
	FST6_KEY_SWA = (1 << 10),
	FST6_KEY_SWB = (1 << 11),
	FST6_KEY_SWC = (1 << 12),
	FST6_KEY_SWD = (1 << 13),
	FST6_KEY_SELECT = (1 << 14), 
	FST6_KEY_ROTA = (1 << 15),
	FST6_KEY_ROTB = (1 << 16)
} fst6_key_mask_t; 

void fst6_init(void); 
fst6_key_mask_t fst6_read_keys(void); 
void fst6_process_events(void); 
serial_dev_t fst6_get_screen_serial_interface(void); 

#ifdef __cplusplus
}
#endif
