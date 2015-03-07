#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <disp/interface.h>
#include <hid/fst6_keys.h>

typedef enum {
	FST6_STICK0, 
	FST6_STICK1, 
	FST6_STICK2, 
	FST6_STICK3, 
	FST6_STICK4, 
	FST6_STICK5,
	FST6_STICKS_COUNT
} fst6_stick_t; 

void fst6_init(void); 
int16_t fst6_read_key(void); 
uint8_t fst6_key_down(fst6_key_code_t key); 
uint16_t fst6_read_battery_voltage(void); 
uint16_t fst6_read_stick(fst6_stick_t id); 
void fst6_play_tone(uint32_t frequency, uint32_t duration_ms); 
void fst6_write_ppm(uint16_t ch1, uint16_t ch2, uint16_t ch3, 
	uint16_t ch4, uint16_t ch5, uint16_t ch6); 
void fst6_process_events(void); 
block_dev_t fst6_get_storage_device(void); 
int8_t fst6_write_config(const uint8_t *data, uint16_t size);
int8_t fst6_read_config(uint8_t *data, uint16_t size); 
serial_dev_t fst6_get_screen_serial_interface(void); 
fbuf_dev_t fst6_get_screen_framebuffer_interface(void); 

#ifdef __cplusplus
}
#endif
