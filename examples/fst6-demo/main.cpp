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

#define CHANNEL_FLAG_REVERSED (1 << 0)

struct channel_config {
	uint8_t source; 
	uint16_t min; 
	uint16_t max; 
	uint8_t rate; 
	uint8_t exponent; 
	uint8_t flags; 
	int16_t offset; 
}; 

struct mix_config {
	uint8_t enabled; 
	uint8_t master_channel; 
	uint8_t slave_channel; 
	uint8_t pos_mix; 
	uint8_t neg_mix; 
	uint8_t offset; 
}; 

struct stick_config {
	int16_t offset; 
}; 

struct config {
	struct channel_config channels[6]; 
	struct stick_config sticks[4]; 
	struct mix_config mixes[3]; 
	uint16_t checksum; 
}; 

const struct config default_config = {
	.channels = {
		{.source = 0, .min = 1000, .max = 2000, .rate = 100, .exponent = 0, .flags = 0, .offset = 0},
		{.source = 1, .min = 1000, .max = 2000, .rate = 100, .exponent = 0, .flags = 0, .offset = 0},
		{.source = 2, .min = 1000, .max = 2000, .rate = 100, .exponent = 0, .flags = 0, .offset = 0},
		{.source = 3, .min = 1000, .max = 2000, .rate = 100, .exponent = 0, .flags = 0, .offset = 0},
		{.source = 4, .min = 1000, .max = 2000, .rate = 100, .exponent = 0, .flags = 0, .offset = 0},
		{.source = 5, .min = 1000, .max = 2000, .rate = 100, .exponent = 0, .flags = 0, .offset = 0}
	}, 
	.sticks = {
		{.offset = 0},
		{.offset = 0},
		{.offset = 0},
		{.offset = 0}
	},
	.mixes {
		{.enabled = 0, .master_channel = 0, .slave_channel = 1, .pos_mix = 50, .neg_mix = 50, .offset = 0}, 
		{.enabled = 0, .master_channel = 0, .slave_channel = 1, .pos_mix = 50, .neg_mix = 50, .offset = 0}, 
		{.enabled = 0, .master_channel = 0, .slave_channel = 1, .pos_mix = 50, .neg_mix = 50, .offset = 0}
	}, 
	.checksum = 0
}; 

uint16_t fletcher16( uint8_t const *data, size_t bytes ){
	uint16_t sum1 = 0xff, sum2 = 0xff;

	while (bytes) {
		size_t tlen = bytes > 20 ? 20 : bytes;
		bytes -= tlen;
		do {
						sum2 += sum1 += *data++;
		} while (--tlen);
		sum1 = (sum1 & 0xff) + (sum1 >> 8);
		sum2 = (sum2 & 0xff) + (sum2 >> 8);
	}
	/* Second reduction step to reduce sums to 8 bits */
	sum1 = (sum1 & 0xff) + (sum1 >> 8);
	sum2 = (sum2 & 0xff) + (sum2 >> 8);
	return sum2 << 8 | sum1;
}

uint8_t config_valid(struct config *self){
	return self->checksum == fletcher16((uint8_t*)self, sizeof(struct config) - sizeof(self->checksum)); 
}

void config_update_checksum(struct config *self){
	self->checksum = fletcher16((uint8_t*)self, sizeof(struct config) - sizeof(self->checksum));
}

uint16_t config_calc_checksum(struct config *self){
	return fletcher16((uint8_t*)self, sizeof(struct config) - sizeof(self->checksum));
}

struct application {
	struct libk_thread main_thread; 
	struct block_transfer tr; 
	block_dev_t eeprom; 
	struct config conf; 
	uint16_t saved_config_checksum; 
	uint16_t prev_config_checksum; 
	timestamp_t save_config_timeout; 
	uint8_t status; 
}; 

static struct application app; 
static struct gui_data *gui; 

LIBK_THREAD(config_thread){
	
	PT_BEGIN(pt); 
	
	while(1){
		static struct block_device_geometry geom; 
		blk_get_geometry(app.eeprom, &geom); 
		
		//printf("Using storage device %dkb, %d sectors, %d pages/sector, %d bytes per page\n", 
		//	geom.pages * geom.page_size, geom.sectors, geom.pages / geom.sectors, geom.page_size); 
			
		PT_WAIT_UNTIL(pt, app.status & (DEMO_STATUS_WR_CONFIG | DEMO_STATUS_RD_CONFIG)); 
		
		if(app.status & DEMO_STATUS_WR_CONFIG){
			printf("CONF: write\n"); 
			
			PT_WAIT_UNTIL(pt, blk_open(app.eeprom)); 
			
			config_update_checksum(&app.conf); 
			
			app.saved_config_checksum = app.conf.checksum; 
			
			blk_transfer_init(&app.tr, app.eeprom, 0, ((uint8_t*)&app.conf), sizeof(struct config), IO_WRITE); 
			PT_WAIT_UNTIL(pt, blk_transfer_completed(&app.tr)); 
			
			blk_close(app.eeprom); 
			
			printf("CONF: saved\n"); 
			app.status &= ~DEMO_STATUS_WR_CONFIG; 
		} 
		if(app.status & DEMO_STATUS_RD_CONFIG){
			printf("CONF: read\n"); 
			
			PT_WAIT_UNTIL(pt, blk_open(app.eeprom)); 
			
			blk_transfer_init(&app.tr, app.eeprom, 0, ((uint8_t*)&app.conf), sizeof(struct config), IO_READ); 
			PT_WAIT_UNTIL(pt, blk_transfer_completed(&app.tr)); 
			
			printf("CONF: loaded\n"); 
			
			// set the saved checksum to the newly loaded config
			app.saved_config_checksum = app.conf.checksum; 
			gui->channel.request_load = 1; 
			
			//config_update_checksum(&app.conf); 
			
			blk_close(app.eeprom); 
			
			app.status &= ~DEMO_STATUS_RD_CONFIG;
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

LIBK_THREAD(output_thread){
	PT_BEGIN(pt); 
	
	while(1){
		// read raw values for each channel
		uint16_t outputs[6]; 
		uint16_t inputs[6]; 
		
		// read all of the input values and update gui inputs indicators
		for(int c = 0; c < 6; c++){
			inputs[c] = map(fst6_read_stick((fst6_stick_t)c), 0, 4096, 0, 1000); 
			gui->inputs[c].value = 1000 + inputs[c]; 
		}
		
		for(int c = 0; c < 6; c++) {
			uint16_t raw = 0; 
			struct channel_config *cc = &app.conf.channels[c]; 
			switch(cc->source){
				case 0: 
				case 1: 
				case 2: 
				case 3: 
				case 4: 
				case 5: {
					if(cc->flags & CHANNEL_FLAG_REVERSED){
						raw = 2000 - inputs[cc->source]; 
					} else {
						raw = 1000 + inputs[cc->source]; 
					}
					if(cc->source < 4)
						raw += app.conf.sticks[cc->source].offset; 
				} break; 
				case 6: 
					raw = (fst6_key_down(FST6_KEY_SWA))?1000:2000; break; 
				case 7: 
					raw = (fst6_key_down(FST6_KEY_SWB))?1000:2000; break; 
				case 8: 
					raw = (fst6_key_down(FST6_KEY_SWC))?1000:2000; break; 
				case 9: 
					raw = (fst6_key_down(FST6_KEY_SWD))?1000:2000; break; 
				default: 
					raw = 1500; 
			}
			outputs[c] = cc->offset + map(raw, cc->min, cc->max, 1000, 2000); 
			// update currently edited gui channel
			if(gui->channel.id == c){
				gui->channel.input = raw; 
				gui->channel.output = outputs[c]; 
			}
		}
		
		// do mixing here
		for(int c = 0; c < 3; c++){
			uint8_t master_id = app.conf.mixes[c].master_channel; 
			uint8_t slave_id = app.conf.mixes[c].slave_channel; 
			if(app.conf.mixes[c].enabled && master_id < 6 && slave_id < 6 && master_id != slave_id){
				int32_t val = outputs[master_id] - 1500; 
				if(val > 0)
					outputs[slave_id] = constrain(outputs[slave_id] + (app.conf.mixes[c].pos_mix * val) / 100 + app.conf.mixes[c].offset * 5, 1000, 2000); 
				else
					outputs[slave_id] = constrain(outputs[slave_id] + (app.conf.mixes[c].neg_mix * val) / 100 - app.conf.mixes[c].offset * 5, 1000, 2000); 
			}
		}
		
		// update gui outputs
		for(int c = 0; c < 6; c++){
			gui->out[c].value = outputs[c]; 
		}
		// write outputs
		fst6_write_ppm(outputs[0], outputs[1], outputs[2], outputs[3], outputs[4], outputs[5]); 
			
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

LIBK_THREAD(main_thread){
	//serial_dev_t screen = fst6_get_screen_serial_interface(); 
	//fst6_key_mask_t keys = fst6_read_keys(); 
	int16_t key = 0; 
	static const int OFFSET_STEP = 5; 
	
	PT_BEGIN(pt); 
	
	// load the config and wait for it to be loaded
	app.status |= DEMO_STATUS_RD_CONFIG; 
	PT_WAIT_WHILE(pt, app.status & DEMO_STATUS_RD_CONFIG); 
	if(!config_valid(&app.conf)){
		printf("CONF: invalid checksum %x, expected %x\n", app.conf.checksum, config_calc_checksum(&app.conf));
		for(uint16_t c = 0; c < sizeof(struct config); c++){
			printf("%x ", ((uint8_t*)&app.conf)[c]); 
		} printf("\n"); 
		memcpy(&app.conf, &default_config, sizeof(struct config)); 
		config_update_checksum(&app.conf); 
		app.status |= DEMO_STATUS_WR_CONFIG; 
		PT_WAIT_WHILE(pt, app.status & DEMO_STATUS_WR_CONFIG); 
	}
	
	while(1){
		// check if it is time to save the config
		config_update_checksum(&app.conf); 
		
		// if the checksum has changed then we set a timeout 
		if(app.conf.checksum != app.prev_config_checksum){
			app.save_config_timeout = timestamp_from_now_us(5000000); 
			app.prev_config_checksum = app.conf.checksum; 
		} 
		// if the checksum is not the same as the saved checksum and 
		// the timeout expired then the config is saved. 
		// this means that we only save the config if it is 5 seconds old and 
		// is different from the one already in the eeprom
		if(app.conf.checksum != app.saved_config_checksum &&
			timestamp_expired(app.save_config_timeout)){
			app.status |= DEMO_STATUS_WR_CONFIG; 
		}
		
		// read switches
		gui->sw[0] = fst6_key_down(FST6_KEY_SWA); 
		gui->sw[1] = fst6_key_down(FST6_KEY_SWB); 
		gui->sw[2] = fst6_key_down(FST6_KEY_SWC); 
		gui->sw[3] = fst6_key_down(FST6_KEY_SWD); 
		
		// sync the gui
		if(gui->channel.request_load){
			uint8_t id = (gui->channel.id < 6)?gui->channel.id:0; 
			struct channel_config *ch = &app.conf.channels[id]; 
			gui->channel.source = ch->source; 
			gui->channel.reverse = (ch->flags & CHANNEL_FLAG_REVERSED)?1:0; 
			gui->channel.rate = ch->rate; 
			gui->channel.exponent = ch->exponent; 
			gui->channel.min = ch->min; 
			gui->channel.max = ch->max; 
			gui->channel.offset = 1500 + ch->offset; 
			gui->channel.request_load = 0; 
		} else {
			uint8_t id = (gui->channel.id < 6)?gui->channel.id:0; 
			struct channel_config *ch = &app.conf.channels[id]; 
			ch->source = gui->channel.source % 10; 
			if(gui->channel.reverse)
				ch->flags |= CHANNEL_FLAG_REVERSED;
			else
				ch->flags &= ~CHANNEL_FLAG_REVERSED; 
			ch->rate = constrain((int8_t)gui->channel.rate, 0, 100); 
			ch->exponent = constrain((int8_t)gui->channel.exponent, 0, 100); 
			ch->min = gui->channel.min; 
			ch->max = gui->channel.max; 
			ch->offset = constrain((int16_t)gui->channel.offset, 1000, 2000) - 1500; 
		}
		
		if(gui->mix.request_load){
			uint8_t id = (gui->mix.id < 3)?gui->mix.id:0; 
			struct mix_config *m = &app.conf.mixes[id]; 
			gui->mix.master_channel = m->master_channel; 
			gui->mix.slave_channel = m->slave_channel; 
			gui->mix.pos_mix = m->pos_mix; 
			gui->mix.neg_mix = m->neg_mix; 
			gui->mix.offset = m->offset; 
			gui->mix.request_load = 0; 
		} else {
			uint8_t id = (gui->mix.id < 3)?gui->mix.id:0; 
			struct mix_config *m = &app.conf.mixes[id]; 
			m->master_channel = gui->mix.master_channel; 
			m->slave_channel = gui->mix.slave_channel; 
			m->pos_mix = gui->mix.pos_mix; 
			m->neg_mix = gui->mix.neg_mix; 
			m->offset = gui->mix.offset; 
		}
		
		// enable any mixes
		for(int c = 0; c < 3; c++){
			if(gui->mix_enabled[c]){
				app.conf.mixes[c].enabled = 1; 
			} else {
				app.conf.mixes[c].enabled = 0; 
			}
		}
		
		while((key = fst6_read_key()) > 0){
			if(!(FST6_KEY_FLAG_UP & key)){
				switch(key & FST6_KEY_MASK){
					case FST6_KEY_OK: 
						m2_fb_put_key(M2_KEY_SELECT); 
						break; 
					case FST6_KEY_SELECT: 
						m2_fb_put_key(M2_KEY_EXIT); 
						break; 
					case FST6_KEY_CANCEL: 
						m2_fb_put_key(M2_KEY_DATA_DOWN); 
						break; 
					case FST6_KEY_ROTA: 
						m2_fb_put_key(M2_KEY_NEXT); 
						break; 
					case FST6_KEY_ROTB: 
						m2_fb_put_key(M2_KEY_PREV); 
						break; 
					case FST6_KEY_CH1P: 
						app.conf.sticks[0].offset += OFFSET_STEP; 
						break; 
					case FST6_KEY_CH1M: 
						app.conf.sticks[0].offset-= OFFSET_STEP; 
						break; 
					case FST6_KEY_CH2P: 
						app.conf.sticks[1].offset+= OFFSET_STEP; 
						break; 
					case FST6_KEY_CH2M: 
						app.conf.sticks[1].offset-= OFFSET_STEP; 
						break; 
					case FST6_KEY_CH3P: 
						app.conf.sticks[2].offset+= OFFSET_STEP; 
						break; 
					case FST6_KEY_CH3M: 
						app.conf.sticks[2].offset-= OFFSET_STEP; 
						break; 
					case FST6_KEY_CH4P: 
						app.conf.sticks[3].offset+= OFFSET_STEP; 
						break; 
					case FST6_KEY_CH4M: 
						app.conf.sticks[3].offset-= OFFSET_STEP; 
						break; 
				}
				printf("KEY: %d\n", key & FST6_KEY_MASK); 
			} else {
				printf("DOW: %d\n", key & FST6_KEY_MASK); 
				/*switch(key & FST6_KEY_MASK){
					case FST6_KEY_SWA: 
						gui->sw[0] = 1; 
						break; 
					case FST6_KEY_SWB: 
						gui->sw[1] = 1; 
						break; 
					case FST6_KEY_SWC: 
						gui->sw[2] = 1; 
						break; 
					case FST6_KEY_SWD: 
						gui->sw[3] = 1; 
						break; 
				}*/
			}
		}
		/*
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
		*/
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
	
	gui = gui_get_data(); 
	
	//status = DEMO_STATUS_WR_CONFIG | DEMO_STATUS_RD_CONFIG; 
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
