#include <arch/soc.h>
#include "m2_tty.h"
#include <util/cbuf.h>
#include <string.h>

static serial_dev_t screen = 0; 
#define LCD_PRINTF(...) serial_printf(screen, __VA_ARGS__)

static struct cbuf key_buffer; 
static uint8_t _key_buffer_data[8]; 

static uint8_t m2_es_custom(m2_p ep, uint8_t msg){
	(void)ep; 
	switch(msg){
		case M2_ES_MSG_GET_KEY:
		{
			if(cbuf_get_waiting(&key_buffer))
				return M2_KEY_EVENT(cbuf_get(&key_buffer)); 
			/*
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
			return M2_KEY_NONE; 
		}
	}
	return 0; 
}

/* x,y origin is at lower left (m2tklib system) */
static void m2_gh_tty_draw_text(uint8_t x, uint8_t y, const char *text)
{
	if(y > 7) y = 7; 
	if(x > 21) x = 21; 
	LCD_PRINTF("\x1b[%d;%dH%s", 8 - y, x + 1, text);
}
/*
static void m2_gh_tty_out(char c)
{
  LCD_PRINTF("%c", c);
}

static void m2_gh_tty_cr(void)
{
  LCD_PRINTF("\n");
}*/
static void m2_gh_tty_clear_screen(void){
	LCD_PRINTF("\x1b[2J"); 
}

/*
uint8_t m2_gh_u8g_bfs(m2_gfx_arg_p  arg)
{
  switch(arg->msg)
  {
    case M2_GFX_MSG_DRAW_NORMAL_NO_FOCUS:
      if ( (arg->font & 4) != 0 )
      {
      	m2_u8g_draw_frame_shadow(arg->x+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->y, arg->w-2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->h);
      }

      m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
      if ( m2_u8g_dev_variables.m2_gh_u8g_invert_at_depth < m2_u8g_dev_variables.m2_gh_u8g_current_depth )
      {
        m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
      }
      break;
    case M2_GFX_MSG_DRAW_NORMAL_PARENT_FOCUS:
      break;
    case M2_GFX_MSG_DRAW_NORMAL_FOCUS:
      if ( (arg->font & 4) != 0 )
      {
        m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
        m2_u8g_draw_box(arg->x+1, arg->y+1, arg->w-2, arg->h-2);
      	m2_u8g_draw_frame_shadow(arg->x+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->y, arg->w-2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->h);
      }
      else
      {
        m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
        m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h);
      }
      m2_u8g_dev_variables.m2_gh_u8g_invert_at_depth = m2_u8g_dev_variables.m2_gh_u8g_current_depth;
      // printf("invert %d, width %d x:%d y:%d\n", m2_gh_u8g_invert_at_depth, arg->w, arg->x, arg->y);
      break;
    case M2_GFX_MSG_DRAW_SMALL_FOCUS:
      m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
      m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h);
      break;
    case M2_GFX_MSG_DRAW_GO_UP:
      //does not work because of missing xor...
       //  also: can not be fixed with FRAME_DRAW_AT_END
       m2_u8g_current_text_color = m2_u8g_bg_text_color; 
      m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h/2);
      break;
    case M2_GFX_MSG_GET_NORMAL_BORDER_HEIGHT:
      if ( (arg->font & 4) != 0 )
	return 3;
      return 0;
    case M2_GFX_MSG_GET_NORMAL_BORDER_WIDTH:
      if ( (arg->font & 4) != 0 )
	return 3+2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size+2*m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size;
      return 2*m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size;
    case M2_GFX_MSG_GET_NORMAL_BORDER_X_OFFSET:
      if ( (arg->font & 4) != 0 )
	return 1+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size+m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size;
      return m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size;
    case M2_GFX_MSG_GET_NORMAL_BORDER_Y_OFFSET:
      if ( (arg->font & 4) != 0 )
	return 2;
      return 0;
    case M2_GFX_MSG_GET_LIST_OVERLAP_HEIGHT:
      return 0;
    case M2_GFX_MSG_GET_LIST_OVERLAP_WIDTH:
      return 0;
    case M2_GFX_MSG_IS_FRAME_DRAW_AT_END:
      return 0; // focus (highlight) is drawn first, then the text string 
  }

  return m2_gh_u8g_base(arg);
}
*/

static uint8_t m2_gh_tty(m2_gfx_arg_p  arg){
	switch(arg->msg)
	{
		case M2_GFX_MSG_INIT:
			break;
		case M2_GFX_MSG_START:
			//m2_gh_tty_small_cursor_pos = 255;
			LCD_PRINTF("\x1b?7l"); // turn off cursor wrapping
			m2_gh_tty_clear_screen();
			break;
		case M2_GFX_MSG_END:
			// reset colors
			LCD_PRINTF("\x1b[m"); 
			//m2_gh_tty_show();
			break;
		case M2_GFX_MSG_DRAW_TEXT:
			m2_gh_tty_draw_text(arg->x, arg->y, arg->s);
			return 0;
		case M2_GFX_MSG_DRAW_HLINE: 
			for(int c = 0; c < arg->w; c++)
				m2_gh_tty_draw_text(arg->x + c, arg->y, "-");
			return 0; 
		case M2_GFX_MSG_DRAW_VLINE: 
			for(int c = 0; c < arg->h; c++)
				m2_gh_tty_draw_text(arg->x, arg->y + c, "|");
			return 0; 
		case M2_GFX_MSG_DRAW_BOX: 
			for(int c = 0; c < arg->w; c++)
				m2_gh_tty_draw_text(arg->x + c, arg->y, "-");
			for(int c = 0; c < arg->w; c++)
				m2_gh_tty_draw_text(arg->x + c, arg->y + arg->h, "-");
			for(int c = 0; c < arg->w; c++)
				m2_gh_tty_draw_text(arg->x, arg->y + c, "|");
			for(int c = 0; c < arg->w; c++)
				m2_gh_tty_draw_text(arg->x + arg->w, arg->y + c, "-");
			return 0; 
		case M2_GFX_MSG_DRAW_NORMAL_PARENT_FOCUS:
		case M2_GFX_MSG_DRAW_NORMAL_FOCUS:
			LCD_PRINTF("\x1b[30;47m"); 
			// draw a full line of inverted selection 
			for(int c = 0; c < arg->w; c++)
				m2_gh_tty_draw_text(arg->x + c, arg->y, " ");
			return 0; 
		case M2_GFX_MSG_DRAW_NORMAL_DATA_ENTRY: 
			LCD_PRINTF("\x1b[30;47m"); 
			return 0; 
		case M2_GFX_MSG_DRAW_NORMAL_NO_FOCUS: 
			LCD_PRINTF("\x1b[37;40m"); 
			return 0;
		case M2_GFX_MSG_DRAW_GO_UP:
			m2_gh_tty_draw_text(arg->x-1, arg->y, "<");
			m2_gh_tty_draw_text(arg->x+arg->w, arg->y, ">");
			return 0;
		case M2_GFX_MSG_DRAW_SMALL_FOCUS:
			//m2_gh_tty_small_cursor_pos = HEIGHT -  1 - arg->y;
			//m2_gh_tty_small_cursor_pos*= WIDTH;
			//m2_gh_tty_small_cursor_pos += arg->x+1;  // why is +1 required here???
			LCD_PRINTF("\x1b[30;47m"); 
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
			return 21;
		case M2_GFX_MSG_GET_DISPLAY_HEIGHT:
			return 8;
		case M2_GFX_MSG_DRAW_VERTICAL_SCROLL_BAR:
			/* scroll bar: "total" total number of items */
			/* scroll bar: "top" topmost item (first visible item) 0 .. total-visible*/
			/* scroll bar: "visible" number of visible items 0 .. total-1 */
			LCD_PRINTF("\x1b[0m"); // reset the colors
			if(arg->x > 20) arg->x = 20; 
			for(int c = 0; c < arg->h; c++)
				m2_gh_tty_draw_text(arg->x, arg->y + c, "|");
			{
				uint16_t h, y;
				h = m2_utl_sb_get_slider_height(arg->h - 1, arg->total, arg->visible);
				y = m2_utl_sb_get_slider_position(arg->h, h, arg->total, arg->visible, arg->top);
				
				//printf("Scroll: x: %d y: %d w: %d h: %d sh: %d sy: %d\n", arg->x, arg->y, arg->w, arg->h, h, y); 
				for(int c = 0; c < h; c++)
					m2_gh_tty_draw_text(arg->x, arg->y + arg->h - y - 1 - c, "#");
				//m2_u8g_draw_box(arg->x+1, arg->y+arg->h-1-h-y, arg->w-2, h);
			}
			return 0;
	}
	return m2_gh_dummy(arg);
}

LIBK_THREAD(m2_tty){
	PT_BEGIN(pt); 
	
	while(1){
		static timestamp_t time = 0; 
		if(timestamp_expired(time)){
			m2_CheckKey();
			m2_HandleKey();  
			m2_Draw();
			time = timestamp_from_now_us(1000000/25); 
		}
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

void m2_put_key(uint8_t key){
	cbuf_put(&key_buffer, key); 
}

void m2_tty_init(serial_dev_t _screen, m2_rom_void_p element){
	screen = _screen; 
	m2_Init(element, m2_es_custom, m2_eh_6bs, m2_gh_tty);
	cbuf_init(&key_buffer, _key_buffer_data, sizeof(_key_buffer_data)); 
}
