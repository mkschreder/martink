/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>
#include <gui/m2_tty.h>

//static int _key_state[32] = {0}; 

extern char _sdata; 

#define DEMO_STATUS_RD_CONFIG (1 << 0)
#define DEMO_STATUS_WR_CONFIG (1 << 1)

//static uint8_t config[128] = {0}; 
static uint8_t status = 0; 

struct m2_tty tty; 

//static struct application app; 
/*
LIBK_THREAD(_main_thread){
	
	PT_BEGIN(pt); 
	
	while(1){
		static struct block_device_geometry geom; 
		blk_get_geometry(app.eeprom, &geom); 
		
		printf("Using storage device %dkb, %d sectors, %d pages/sector, %d bytes per page\n", 
			geom.pages * geom.page_size, geom.sectors, geom.pages / geom.sectors, geom.page_size); 
			
		PT_WAIT_UNTIL(pt, status & (DEMO_STATUS_WR_CONFIG | DEMO_STATUS_RD_CONFIG)); 
		
		if(status & DEMO_STATUS_WR_CONFIG){
			printf("Preparing to write test data\n"); 
			
			for(uint16_t c = 0; c < sizeof(config) / geom.page_size; c++){
				memset(config + (c * geom.page_size), c, geom.page_size); 
			}
			
			PT_WAIT_UNTIL(pt, blk_open(app.eeprom)); 
			
			static uint16_t j = 0; 
			for(j = 0; j < (sizeof(config) / geom.page_size); j++){
				printf("Writing page #%d\n", j); 
				blk_transfer_init(&app.tr, app.eeprom, (j * geom.page_size), config + (j * geom.page_size), geom.page_size, IO_WRITE); 
				PT_WAIT_UNTIL(pt, blk_transfer_completed(&app.tr)); 
			}
			
			blk_close(app.eeprom); 
			
			printf("Test data written!\n"); 
			status &= ~DEMO_STATUS_WR_CONFIG; 
		} 
		if(status & DEMO_STATUS_RD_CONFIG){
			printf("Reading back test data... \n"); 
			
			memset(config, 0, sizeof(config)); 
			
			PT_WAIT_UNTIL(pt, blk_open(app.eeprom)); 
			
			static uint16_t j = 0; 
			for(j = 0; j < (sizeof(config) / geom.page_size); j++){
				blk_transfer_init(&app.tr, app.eeprom, (j * geom.page_size), config + (j * geom.page_size), geom.page_size, IO_READ); 
				PT_WAIT_UNTIL(pt, blk_transfer_completed(&app.tr)); 
			}
			
			printf("Checking data..\n"); 
			
			for(j = 0; j < (sizeof(config) / geom.page_size); j++){
				int page_valid = 1; 
				uint16_t c = 0; 
				for(c = 0; c < geom.page_size; c++){
					if(config[(j * geom.page_size) + c] != j){
						page_valid = 0; 
						break; 
					}
				}
				if(!page_valid){
					printf("Page %d is invalid at byte %d!\n", j, c); 
				} else {
					printf("Page %d: success!\n", j); 
				}
			}
			printf("All done!\n"); 
			
			blk_close(app.eeprom); 
			
			status &= ~DEMO_STATUS_RD_CONFIG;
		}
	}
	
	PT_END(pt); 
}*/
/*
LIBK_THREAD(_test_thread){
	PT_BEGIN(pt); 
	
	printf("now writing config...\n"); 
	status |= DEMO_STATUS_WR_CONFIG; 
	memcpy(config, "Blah blah", 9); 
	PT_WAIT_UNTIL(pt, !(status & DEMO_STATUS_WR_CONFIG)); 
	
	//status |= DEMO_STATUS_RD_CONFIG; 
	//PT_WAIT_UNTIL(pt, !(status & DEMO_STATUS_RD_CONFIG)); 
	
	printf("now reading back..\n"); 
	memset(config, 0, 15); 
	status |= DEMO_STATUS_RD_CONFIG; 
	PT_WAIT_UNTIL(pt, !(status & DEMO_STATUS_RD_CONFIG)); 
	printf("Data: %s\n", config);
	
	PT_WAIT_WHILE(pt, 1); 
	
	PT_END(pt); 
}
*/
/*
LIBK_THREAD(_console){
	static serial_dev_t serial = 0; 
	if(!serial) serial = uart_get_serial_interface(0); 
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_WHILE(pt, uart_getc(0) == SERIAL_NO_DATA); 
		libk_print_info(); 
		PT_YIELD(pt); 
	}
	PT_END(pt); 
}*/

#include "gui.c"

/*======================================================================*/

void generic_root_change_cb(m2_rom_void_p new_root, m2_rom_void_p old_root, uint8_t change_value)
{
  printf("%p->%p %d\n", old_root, new_root, change_value);
}

serial_dev_t screen = 0; 

LIBK_THREAD(_demo_thread){
	//serial_dev_t screen = fst6_get_screen_serial_interface(); 
	//fst6_key_mask_t keys = fst6_read_keys(); 
	//static uint16_t sticks[6]; 
	
	PT_BEGIN(pt); 
	
	while(1){
		int16_t key = 0; 
		while((key = fst6_read_key()) > 0){
			if(!(FST6_KEY_FLAG_UP & key)){
				switch(key & FST6_KEY_MASK){
					case FST6_KEY_OK: 
						m2_tty_put_key(&tty, M2_KEY_SELECT); 
						break; 
					case FST6_KEY_SELECT: 
						m2_tty_put_key(&tty, M2_KEY_EXIT); 
						break; 
					case FST6_KEY_CANCEL: 
						m2_tty_put_key(&tty, M2_KEY_DATA_DOWN); 
						break; 
					case FST6_KEY_ROTA: 
						m2_tty_put_key(&tty, M2_KEY_NEXT); 
						break; 
					case FST6_KEY_ROTB: 
						m2_tty_put_key(&tty, M2_KEY_PREV); 
						break; 
				}
				
				printf("KEY: %d\n", key & FST6_KEY_MASK); 
			} else {
				
				printf("DOW: %d\n", key & FST6_KEY_MASK); 
				
			}
		}
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

int main(void){
	fst6_init(); 
	
	printf("SystemCoreClock: %d\n", (int)SystemCoreClock); 
	
	//app.eeprom = fst6_get_storage_device(); 
	screen = fst6_get_screen_serial_interface(); 
	
	m2_tty_init(&tty, screen, 21, 8, &top_el_tlsm); 
	
	//m2_Init(&m2_root, m2_es_custom, m2_eh_4bs, m2_gh_tty);

	status = DEMO_STATUS_WR_CONFIG | DEMO_STATUS_RD_CONFIG; 
	
	timestamp_delay_us(500000); 
	
	printf("Running libk loop\n"); 
	
	libk_run(); 
}


/*
{
			timestamp_t t = timestamp_now(); 
		
			serial_printf(screen, "\x1b[2J\x1b[1;1H"); 
			serial_printf(screen, " FlySky FS-T6 %dMhz\n", (SystemCoreClock / 1000000UL)); 
			
			//serial_printf(screen, "%s\n", (char*)buf); 
			
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
			t = timestamp_ticks_to_us(timestamp_now() - t); 
			serial_printf(screen, "f:%lu,t:%d\n", libk_get_fps(), (uint32_t)t); 
		}
		PT_YIELD(pt); 
fst6_key_mask_t keys = fst6_read_keys(); 
			static uint32_t pressed = 0; 
			for(int c = 0; c < 32; c++){
				// play key sounds. 25ms long, 300hz
				if(keys & (1 << c) && !(pressed & (1 << c))){// key is pressed 
					pressed |= (1 << c); 
					switch(c){
						case 8: 
							return M2_KEY_EVENT(M2_KEY_SELECT); 
						case 9: 
							return M2_KEY_EVENT(M2_KEY_EXIT); 
						case 16: 
							if(pressed & (1 << 15)) {
								return M2_KEY_EVENT(M2_KEY_PREV); 
							}
							else {
								return M2_KEY_EVENT(M2_KEY_NEXT); 
							}
					}
				} else if(!(keys & (1 << c)) && (pressed & (1 << c))){ // released
					pressed &= ~(1 << c); 
				}
			}*/
