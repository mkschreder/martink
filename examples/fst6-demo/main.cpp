/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>

static int _key_state[32] = {0}; 

extern char _sdata; 

#define DEMO_STATUS_RD_CONFIG (1 << 0)
#define DEMO_STATUS_WR_CONFIG (1 << 1)

static uint8_t config[16] = {0}; 
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

struct application {
	struct libk_thread main_thread; 
	struct block_transfer tr; 
	block_dev_t eeprom; 
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

static struct application app; 

LIBK_THREAD(_main_thread){
	PT_BEGIN(pt); 
	
	while(1){
		PT_WAIT_UNTIL(pt, status & (DEMO_STATUS_WR_CONFIG | DEMO_STATUS_RD_CONFIG)); 
		
		if(status & DEMO_STATUS_WR_CONFIG){
			printf("Writing config... \n"); 
			
			strncpy((char*)config, "Hello World!", 16); 
			
			PT_WAIT_UNTIL(pt, blk_open(app.eeprom)); 
			
			blk_transfer_init(&app.tr, app.eeprom, 32, config, sizeof(config), IO_WRITE); 
			PT_WAIT_UNTIL(pt, blk_transfer_completed(&app.tr)); 
			
			blk_close(app.eeprom); 
			
			printf("Config written successfully\n"); 
			status &= ~DEMO_STATUS_WR_CONFIG; 
		} 
		if(status & DEMO_STATUS_RD_CONFIG){
			printf("Reading config... \n"); 
			
			memset(config, 0, sizeof(config)); 
			
			PT_WAIT_UNTIL(pt, blk_open(app.eeprom)); 
			
			blk_transfer_init(&app.tr, app.eeprom, 32, config, sizeof(config), IO_READ); 
			PT_WAIT_UNTIL(pt, blk_transfer_completed(&app.tr)); 
			
			printf("Done!\n"); 
			printf(": %s\n", config); 
			
			blk_close(app.eeprom); 
			
			status &= ~DEMO_STATUS_RD_CONFIG;
		}
	}
	
	PT_END(pt); 
}
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

LIBK_THREAD(_demo_thread){
	serial_dev_t screen = fst6_get_screen_serial_interface(); 
	fst6_key_mask_t keys = fst6_read_keys(); 
	static uint16_t sticks[6]; 
	
	PT_BEGIN(pt); 
	
	while(1){
		serial_printf(screen, "\x1b[2J\x1b[1;1H"); 
		serial_printf(screen, " FlySky FS-T6 %dMhz\n", (SystemCoreClock / 1000000UL)); 
		serial_printf(screen, " FPS: %lu\n", libk_get_fps()); 
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
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

int main(void){
	fst6_init(); 
	
	printf("SystemCoreClock: %d\n", (int)SystemCoreClock); 
	
	app.eeprom = fst6_get_storage_device(); 
	
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
