/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>
#include <lib/m2gui/m2.h>

static int _key_state[32] = {0}; 

extern char _sdata; 

#define DEMO_STATUS_RD_CONFIG (1 << 0)
#define DEMO_STATUS_WR_CONFIG (1 << 1)

static uint8_t config[128] = {0}; 
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

static uint32_t number = 123; 

M2_LABEL(el_label, NULL, "Num: ");
M2_U32NUM(el_num, "a0c4", &number);
M2_BUTTON(el_ok, "", " ok ", NULL);
M2_LIST(list) = { &el_label, &el_num, &el_ok};
M2_HLIST(m2_root, NULL, list);

static serial_dev_t screen; 
#define LCD_PRINTF(...) serial_printf(screen, __VA_ARGS__)

uint8_t m2_es_custom(m2_p ep, uint8_t msg){
	(void)ep; 
	switch(msg){
		case M2_ES_MSG_GET_KEY:
		{
			fst6_key_mask_t keys = fst6_read_keys(); 
			static uint32_t pressed = 0; 
			for(int c = 0; c < 32; c++){
				// play key sounds. 25ms long, 300hz
				if(keys & (1 << c) && !(pressed & (1 << c))){// key is pressed 
					pressed |= (1 << c); 
					switch(c){
						case 8: 
							printf("SELECT\n"); 
							return M2_KEY_EVENT(M2_KEY_SELECT); 
						case 9: 
							printf("EXIT\n"); 
							return M2_KEY_EVENT(M2_KEY_EXIT); 
						case 16: 
							if(pressed & (1 << 15)) {
								printf("PREV\n"); 
								return M2_KEY_EVENT(M2_KEY_PREV); 
							}
							else {
								printf("NEXT\n"); 
								return M2_KEY_EVENT(M2_KEY_NEXT); 
							}
					}
				} else if(!(keys & (1 << c)) && (pressed & (1 << c))){ // released
					pressed &= ~(1 << c); 
				}
			}
			return M2_KEY_NONE; 
		}
	}
	return 0; 
}

/* x,y origin is at lower left (m2tklib system) */
void m2_gh_tty_draw_text(uint8_t x, uint8_t y, const char *text)
{
	LCD_PRINTF("\x1b[%d;%dH%s", y+1, x+1, text);
	/*
  uint8_t idx;
  char *ptr = m2_gh_tty_screen;
  idx = HEIGHT - 1;
  idx -= y;
  idx *= WIDTH;
  idx += x;
  ptr += idx;
  while( *text != '\0' )
    *ptr++ = *text++;
   */
}

void m2_gh_tty_out(char c)
{
  LCD_PRINTF("%c", c);
}

void m2_gh_tty_cr(void)
{
  LCD_PRINTF("\n");
}
void m2_gh_tty_clear_screen(void){
	LCD_PRINTF("\x1b[2J"); 
}

uint8_t m2_gh_tty(m2_gfx_arg_p  arg){
	switch(arg->msg)
	{
		case M2_GFX_MSG_INIT:
			break;
		case M2_GFX_MSG_START:
			//m2_gh_tty_small_cursor_pos = 255;
			m2_gh_tty_clear_screen();
			break;
		case M2_GFX_MSG_END:
			//m2_gh_tty_show();
			break;
		case M2_GFX_MSG_DRAW_TEXT:
			m2_gh_tty_draw_text(arg->x, arg->y, arg->s);
			return 0;
		case M2_GFX_MSG_DRAW_NORMAL_FOCUS:
		case M2_GFX_MSG_DRAW_NORMAL_PARENT_FOCUS:
			m2_gh_tty_draw_text(arg->x, arg->y, "[");
			m2_gh_tty_draw_text(arg->x+arg->w-1, arg->y, "]");
			return 0;
		case M2_GFX_MSG_DRAW_GO_UP:
			m2_gh_tty_draw_text(arg->x-1, arg->y, "<");
			m2_gh_tty_draw_text(arg->x+arg->w, arg->y, ">");
			return 0;
		case M2_GFX_MSG_DRAW_SMALL_FOCUS:
			//m2_gh_tty_small_cursor_pos = HEIGHT -  1 - arg->y;
			//m2_gh_tty_small_cursor_pos*= WIDTH;
			//m2_gh_tty_small_cursor_pos += arg->x+1;  // why is +1 required here???
			return 0;
		case M2_GFX_MSG_DRAW_ICON:
			if ( arg->icon == M2_ICON_TOGGLE_ACTIVE || arg->icon == M2_ICON_RADIO_ACTIVE )
				m2_gh_tty_draw_text(arg->x, arg->y, "*");
			else
				m2_gh_tty_draw_text(arg->x, arg->y, ".");
			return 0;
		case M2_GFX_MSG_GET_TEXT_WIDTH:
			return strlen(arg->s);
		case M2_GFX_MSG_GET_ICON_WIDTH:
		case M2_GFX_MSG_GET_ICON_HEIGHT:
		case M2_GFX_MSG_GET_NUM_CHAR_WIDTH:
		case M2_GFX_MSG_GET_CHAR_WIDTH:
			return 1; 
		case M2_GFX_MSG_GET_CHAR_HEIGHT:
			return 1;
		case M2_GFX_MSG_GET_NORMAL_BORDER_WIDTH:
			return 2;
		case M2_GFX_MSG_GET_NORMAL_BORDER_X_OFFSET:
			return 1;
		case M2_GFX_MSG_GET_DISPLAY_WIDTH:
			return 22;
		case M2_GFX_MSG_GET_DISPLAY_HEIGHT:
			return 8;
		case M2_GFX_MSG_DRAW_VERTICAL_SCROLL_BAR:
			/* scroll bar: "total" total number of items */
			/* scroll bar: "top" topmost item (first visible item) 0 .. total-visible*/
			/* scroll bar: "visible" number of visible items 0 .. total-1 */

			/* not yet implemented
			{
	uint16_t h, y, byte_index;
	char c;
	h = m2_utl_sb_get_slider_height(arg->h*8-2, arg->total, arg->visible);
	y = m2_utl_sb_get_slider_position(arg->h*8-2, h, arg->total, arg->visible, arg->top); 
	for( byte_index = 0; byte_index < arg->h; byte_index++ )
	{
		m2_gh_lc_set_vsb_char(arg->h*8, 1, y, h, byte_index);
		m2_gh_lc_set_cursor(arg->x, arg->y+arg->h-1-byte_index);
		c = 3;
		c += byte_index;
		m2_lc_ptr->print(c);
	}
			}
			*/
			return 1;
	}
	return m2_gh_dummy(arg);
}

LIBK_THREAD(_demo_thread){
	//serial_dev_t screen = fst6_get_screen_serial_interface(); 
	fst6_key_mask_t keys = fst6_read_keys(); 
	static uint16_t sticks[6]; 
	
	PT_BEGIN(pt); 
	
	while(1){
		m2_CheckKey();
		m2_HandleKey();  
		m2_Draw();
		PT_YIELD(pt); 
		continue; 
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
	}
	
	PT_END(pt); 
}

int main(void){
	fst6_init(); 
	
	printf("SystemCoreClock: %d\n", (int)SystemCoreClock); 
	
	app.eeprom = fst6_get_storage_device(); 
	screen = fst6_get_screen_serial_interface(); 
	
	m2_Init(&m2_root, m2_es_custom, m2_eh_4bs, m2_gh_tty);

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
