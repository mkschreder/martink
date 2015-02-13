/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>

static int _key_state[32] = {0}; 

extern char _sdata; 

int main(void){
	fst6_init(); 
	
	serial_dev_t screen = fst6_get_screen_serial_interface(); 
	
	// test config read/write (to eeprom)
	const char str[] = "Hello World!"; 
	uint8_t buf[13] = {0}; 
	printf("Writing string to config: %s\n", str); 
	fst6_write_config((const uint8_t*)str, sizeof(str)); 
	printf("Reading string from config: "); 
	fst6_read_config(buf, sizeof(str)); 
	printf("%s\n", buf); 
	
	while(1){
		serial_printf(screen, "\x1b[2J\x1b[1;1H"); 
		serial_printf(screen, "    FlySky FS-T6    \n"); 
		serial_printf(screen, " LibK example program\n"); 
		//serial_printf(screen, "%s\n", (char*)buf); 
		uint16_t sticks[6]; 
		for(int c = 0; c < 6; c+=2) {
			sticks[c] = (int)fst6_read_stick((fst6_stick_t)c); 
			sticks[c+1] = (int)fst6_read_stick((fst6_stick_t)(c+1)); 
			sticks[c] = 1000 + (sticks[c] >> 2); 
			sticks[c+1] = 1000 + (sticks[c+1] >> 2); 
			
			serial_printf(screen, "CH%d: %04d CH%d: %04d\n", 
				c, (int)sticks[c], 
				c + 1, (int)sticks[c+1]); 
		}
		// write ppm
		fst6_write_ppm(sticks[0], sticks[1], sticks[2], sticks[3], sticks[4], sticks[5]); 
		
		serial_printf(screen, "VBAT: %d\n", (int)fst6_read_battery_voltage()); 
		
		fst6_key_mask_t keys = fst6_read_keys(); 
		
		serial_printf(screen, "Keys: "); 
		for(int c = 0; c < 32; c++){
			// play key sounds. 25ms long, 300hz
			if(keys & (1 << c) && !_key_state[c]){// key is pressed 
				fst6_play_tone(300, 25); 
				_key_state[c] = 1; 
			} else if(!(keys & (1 << c)) && _key_state[c]){ // released
				_key_state[c] = 0; 
			}
			if(keys & (1 << c)){
				serial_printf(screen, "%d ", c); 
			}
		}
		//serial_printf(screen, "\n"); 
		fst6_process_events(); 
	}
}
