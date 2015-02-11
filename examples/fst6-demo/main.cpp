/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>

int main(void){
	fst6_init(); 
	
	serial_dev_t screen = fst6_get_screen_serial_interface(); 
	
	while(1){
		serial_printf(screen, "\x1b[2J\x1b[1;1H"); 
		serial_printf(screen, "    FlySky FS-T6    \n"); 
		serial_printf(screen, " LibK example program\n"); 
		for(int c = 0; c < 6; c+=2) {
			serial_printf(screen, "CH%d: %04d CH%d: %04d\n", c, (int)adc_read(c), c + 1, (int)adc_read(c+1)); 
		}
		
		fst6_key_mask_t keys = fst6_read_keys(); 
	
		serial_printf(screen, "Keys: "); 
		for(int c = 0; c < 32; c++){
			if(keys & (1 << c)){
				serial_printf(screen, "%d ", c); 
			}
		}
		serial_printf(screen, "\n"); 
		fst6_process_events(); 
	}
}
