/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>

static int _key_state[32] = {0}; 

int main(void){
	fst6_init(); 
	
	serial_dev_t screen = fst6_get_screen_serial_interface(); 
	
	while(1){
		serial_printf(screen, "\x1b[2J\x1b[1;1H"); 
		serial_printf(screen, "    FlySky FS-T6    \n"); 
		serial_printf(screen, " LibK example program\n"); 
		for(int c = 0; c < 6; c+=2) {
			serial_printf(screen, "CH%d: %04d CH%d: %04d\n", 
				c, (int)fst6_read_stick((fst6_stick_t)c), 
				c + 1, (int)fst6_read_stick((fst6_stick_t)(c+1))); 
		}
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
		serial_printf(screen, "\n"); 
		fst6_process_events(); 
	}
}
