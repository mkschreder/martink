#include <arch/soc.h>

#include "m2_tty.h"
#include <string.h>

static struct m2_tty *_m2_tty_current = 0; 

static uint8_t _m2_tty_es(m2_p ep, uint8_t msg){
	(void)ep; 
	struct m2_tty *self = _m2_tty_current; 
	if(!self) return 0; 
	
	switch(msg){
		case M2_ES_MSG_GET_KEY:
		{
			if(cbuf_get_waiting(&self->key_buffer))
				return M2_KEY_EVENT(cbuf_get(&self->key_buffer)); 
			return M2_KEY_NONE; 
		}
	}
	return 0; 
}

/* x,y origin is at lower left (m2tklib system) */
static void _m2_tty_draw_string(struct m2_tty *self, uint8_t x, uint8_t y, const char *text)
{
	if(y > self->height) y = self->height; 
	if(x > self->width) x = self->width; 
	serial_printf(self->screen, "\x1b[%d;%dH%s", self->height - y, x + 1, text);
}

static void _m2_tty_clear_screen(struct m2_tty *self){
	serial_printf(self->screen, "\x1b[2J"); 
}


static uint8_t _m2_tty_gh(m2_gfx_arg_p  arg){
	struct m2_tty *self = _m2_tty_current; //container_of(arg, struct m2_tty, arg); 
	if(!self) return 0; 
	
	switch(arg->msg){
		case M2_GFX_MSG_INIT:
			break;
		case M2_GFX_MSG_START:
			//m2_gh_tty_small_cursor_pos = 255;
			serial_printf(self->screen, "\x1b?7l"); // turn off cursor wrapping
			_m2_tty_clear_screen(self);
			break;
		case M2_GFX_MSG_END:
			// reset colors
			serial_printf(self->screen, "\x1b[m"); 
			//m2_gh_tty_show();
			break;
		case M2_GFX_MSG_DRAW_TEXT:
			_m2_tty_draw_string(self, arg->x, arg->y, arg->s);
			return 0;
		case M2_GFX_MSG_DRAW_HLINE: 
			for(int c = 0; c < arg->w; c++)
				_m2_tty_draw_string(self, arg->x + c, arg->y, "-");
			return 0; 
		case M2_GFX_MSG_DRAW_VLINE: 
			for(int c = 0; c < arg->h; c++)
				_m2_tty_draw_string(self, arg->x, arg->y + c, "|");
			return 0; 
		case M2_GFX_MSG_DRAW_BOX: 
			for(int c = 0; c < arg->w; c++)
				_m2_tty_draw_string(self, arg->x + c, arg->y, "-");
			for(int c = 0; c < arg->w; c++)
				_m2_tty_draw_string(self, arg->x + c, arg->y + arg->h, "-");
			for(int c = 0; c < arg->w; c++)
				_m2_tty_draw_string(self, arg->x, arg->y + c, "|");
			for(int c = 0; c < arg->w; c++)
				_m2_tty_draw_string(self, arg->x + arg->w, arg->y + c, "-");
			return 0; 
		case M2_GFX_MSG_DRAW_NORMAL_PARENT_FOCUS:
		case M2_GFX_MSG_DRAW_NORMAL_FOCUS:
			serial_printf(self->screen, "\x1b[30;47m"); 
			// draw a full line of inverted selection 
			for(int c = 0; c < arg->w; c++)
				_m2_tty_draw_string(self, arg->x + c, arg->y, " ");
			return 0; 
		case M2_GFX_MSG_DRAW_NORMAL_DATA_ENTRY: 
			serial_printf(self->screen, "\x1b[30;47m"); 
			return 0; 
		case M2_GFX_MSG_DRAW_NORMAL_NO_FOCUS: 
			serial_printf(self->screen, "\x1b[37;40m"); 
			return 0;
		case M2_GFX_MSG_DRAW_GO_UP:
			_m2_tty_draw_string(self, arg->x-1, arg->y, "<");
			_m2_tty_draw_string(self, arg->x+arg->w, arg->y, ">");
			return 0;
		case M2_GFX_MSG_DRAW_SMALL_FOCUS:
			//m2_gh_tty_small_cursor_pos = HEIGHT -  1 - arg->y;
			//m2_gh_tty_small_cursor_pos*= WIDTH;
			//m2_gh_tty_small_cursor_pos += arg->x+1;  // why is +1 required here???
			serial_printf(self->screen, "\x1b[30;47m"); 
			return 0;
		case M2_GFX_MSG_DRAW_ICON:
			if ( arg->icon == M2_ICON_TOGGLE_ACTIVE || arg->icon == M2_ICON_RADIO_ACTIVE )
				_m2_tty_draw_string(self, arg->x, arg->y, "*");
			else
				_m2_tty_draw_string(self, arg->x, arg->y, ".");
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
			return self->width;
		case M2_GFX_MSG_GET_DISPLAY_HEIGHT:
			return self->height;
		case M2_GFX_MSG_DRAW_VERTICAL_SCROLL_BAR:
			/* scroll bar: "total" total number of items */
			/* scroll bar: "top" topmost item (first visible item) 0 .. total-visible*/
			/* scroll bar: "visible" number of visible items 0 .. total-1 */
			serial_printf(self->screen, "\x1b[0m"); // reset the colors
			if(arg->x > 20) arg->x = 20; 
			for(int c = 0; c < arg->h; c++)
				_m2_tty_draw_string(self, arg->x, arg->y + c, "|");
			{
				uint16_t h, y;
				h = m2_utl_sb_get_slider_height(arg->h - 1, arg->total, arg->visible);
				y = m2_utl_sb_get_slider_position(arg->h, h, arg->total, arg->visible, arg->top);
				
				//printf("Scroll: x: %d y: %d w: %d h: %d sh: %d sy: %d\n", arg->x, arg->y, arg->w, arg->h, h, y); 
				for(uint16_t c = 0; c < h; c++)
					_m2_tty_draw_string(self, arg->x, arg->y + arg->h - y - 1 - c, "#");
				//m2_u8g_draw_box(arg->x+1, arg->y+arg->h-1-h-y, arg->w-2, h);
			}
			return 0;
	}
	return m2_gh_dummy(arg);
}

static PT_THREAD(_m2_tty_thread(struct libk_thread *kthread, struct pt *pt)){
	struct m2_tty *self = container_of(kthread, struct m2_tty, thread); 
	
	PT_BEGIN(pt); 
	
	while(1){
		_m2_tty_current = self; 
		m2_CheckKey();
		m2_HandleKey();  
		m2_Draw();
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

void m2_tty_put_key(struct m2_tty *self, uint8_t key){
	cbuf_put(&self->key_buffer, key); 
}

void m2_tty_init(struct m2_tty *self, serial_dev_t _screen, uint16_t w, uint16_t h, m2_rom_void_p element){
	self->screen = _screen; 
	self->width = w; 
	self->height = h; 
	
	m2_Init(element, _m2_tty_es, m2_eh_6bs, _m2_tty_gh);
	
	cbuf_init(&self->key_buffer, self->key_buffer_data, sizeof(self->key_buffer_data)); 
	
	libk_create_thread(&self->thread, _m2_tty_thread, "m2_tty"); 
}
