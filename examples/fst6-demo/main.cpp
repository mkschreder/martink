/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>
#include <gui/m2_fb.h>
#include "gui.hpp" 

extern char _sdata; 

#define DEMO_STATUS_RD_CONFIG (1 << 0)
#define DEMO_STATUS_WR_CONFIG (1 << 1)

static uint8_t config[32] = {0}; 
static uint8_t status = 0; 

typedef enum {
	IO_WRITE, 
	IO_READ
} io_direction_t; 

struct block_transfer {
	uint8_t completed; 
	ssize_t address; 
	ssize_t transfered; 
	ssize_t size; 
	uint8_t *buffer; 
	io_direction_t dir; 
	block_dev_t dev; 
}; 

void blk_transfer_init(struct block_transfer *tr, 
	block_dev_t dev, uint32_t address, uint8_t *buffer, uint32_t size, io_direction_t dir){
	tr->completed = 0; 
	tr->address = address; 
	tr->transfered = 0; 
	tr->size = size; 
	tr->buffer = buffer; 
	tr->dir = dir; 
	tr->dev = dev; 
}

uint8_t blk_transfer_completed(struct block_transfer *tr){
	if(tr->completed == 1) {
		 return 1; 
	} else if(tr->transfered < tr->size){
		ssize_t transfered = 0; 
		
		if(tr->dir == IO_WRITE)
			transfered = blk_writepage(tr->dev, tr->address + tr->transfered, tr->buffer + tr->transfered, tr->size - tr->transfered); 
		else
			transfered = blk_readpage(tr->dev, tr->address + tr->transfered, tr->buffer + tr->transfered, tr->size - tr->transfered); 
			
		if(transfered > 0) {
			printf("Transfered %d bytes of %d\n", transfered, tr->size); 
			tr->transfered += transfered; 
		}
	} else if(tr->transfered == tr->size && !blk_get_status(tr->dev, BLKDEV_BUSY)){
		tr->completed = 1; 
		return 1; 
	}
	
	return 0; 
}

struct application {
	struct libk_thread main_thread; 
	struct block_transfer tr; 
	block_dev_t eeprom; 
}; 

static struct application app; 
static struct gui_model *gui; 

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
}

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
}

// valign: bottom, halign: center, display width
/*======================================================================*/

void generic_root_change_cb(m2_rom_void_p new_root, m2_rom_void_p old_root, uint8_t change_value)
{
  printf("%p->%p %d\n", old_root, new_root, change_value);
}

LIBK_THREAD(_demo_thread){
	//serial_dev_t screen = fst6_get_screen_serial_interface(); 
	//fst6_key_mask_t keys = fst6_read_keys(); 

	PT_BEGIN(pt); 
	
	while(1){
		static fst6_key_mask_t keys; 
		static uint32_t pressed = 0; 
		
		keys = fst6_read_keys(); 
		for(int c = 0; c < 32; c++){
			// play key sounds. 25ms long, 300hz
			if(keys & (1 << c) && !(pressed & (1 << c))){// key is pressed 
				pressed |= (1 << c); 
				switch(c){
					case 8: 
						if(pressed & (1 << 14))
							m2_fb_put_key(M2_KEY_DATA_UP); 
						else
							m2_fb_put_key(M2_KEY_SELECT); 
						break; 
					case 9: 
						if(pressed & (1 << 14))
							m2_fb_put_key(M2_KEY_DATA_DOWN); 
						else
							m2_fb_put_key(M2_KEY_EXIT); 
						break; 
					case 16: 
						if(pressed & (1 << 14)){
							if(pressed & (1 << 15)) {
								m2_fb_put_key(M2_KEY_DATA_UP); 
							}
							else {
								m2_fb_put_key(M2_KEY_DATA_DOWN); 
							}
						} else {
							if(pressed & (1 << 15)) {
								m2_fb_put_key(M2_KEY_NEXT); 
							}
							else {
								m2_fb_put_key(M2_KEY_PREV); 
							}
						}
						break; 
				}
				printf("Key: %d\n", c); 
			} else if(!(keys & (1 << c)) && (pressed & (1 << c))){ // released
				pressed &= ~(1 << c); 
			}
		}
		
		gui->sw[0].on = (pressed & (1 << 10)) != 0; 
		gui->sw[1].on = (pressed & (1 << 11)) != 0; 
		gui->sw[2].on = (pressed & (1 << 12)) != 0; 
		gui->sw[3].on = (pressed & (1 << 13)) != 0; 
		
		for(int c = 0; c < 6; c++) {
			gui->ch[c].value = (int)fst6_read_stick((fst6_stick_t)c); 
			
			if(!gui->calibrated){
				gui->ch[c].value = 1000 + (gui->ch[c].value >> 2); 
			
				if(gui->ch[c].value < gui->ch[c].min)
					gui->ch[c].min = gui->ch[c].value; 
				if(gui->ch[c].value > gui->ch[c].max)
					gui->ch[c].max = gui->ch[c].value; 
			} else {
				gui->ch[c].value = map(1000 + (gui->ch[c].value >> 2), 
					gui->ch[c].min, gui->ch[c].max, 1000, 2000); 
			}
		}
		
		PT_YIELD(pt); 
		/*continue; 
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
		PT_YIELD(pt); */
	}
	
	PT_END(pt); 
}

int main(void){
	fst6_init(); 
	
	printf("SystemCoreClock: %d\n", (int)SystemCoreClock); 
	
	app.eeprom = fst6_get_storage_device(); 
	
	gui_init(fst6_get_screen_framebuffer_interface()); 
	
	gui = gui_get_model(); 
	
	status = DEMO_STATUS_WR_CONFIG | DEMO_STATUS_RD_CONFIG; 
	/*
	// test config read/write (to eeprom)
	const char str[] = "Hello World!"; 
	uint8_t buf[13] = {0}; 
	printf("Writing string to config: %s\n", str); 
	fst6_write_config((const uint8_t*)str, sizeof(str)); 
	printf("Reading string from config: "); 
	fst6_read_config(buf, sizeof(str)); 
	printf("%s\n", buf); 
	*/
	
	printf("Running libk loop\n"); 
	
	libk_run(); 
}
