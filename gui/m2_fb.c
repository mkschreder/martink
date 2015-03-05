#include <arch/soc.h>

#include "m2_fb.h"
#include <util/cbuf.h>
#include <string.h>

static struct m2_fb _m2fb; 

static uint8_t _m2_fb_es(m2_p ep, uint8_t msg){
	//struct m2_fb *self = container_of(ep, struct m2_fb, m2); 
	(void)ep; 
	struct m2_fb *self = &_m2fb; 
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

static PT_THREAD(_m2_fb_thread(struct libk_thread *kthread, struct pt *pt)){
	(void)kthread; 
	struct m2_fb *self = &_m2fb; 
	
	//struct m2_fb *self = container_of(kthread, struct m2_fb, thread); 
	
	PT_BEGIN(pt); 
	
	while(1){
		if(timestamp_expired(self->time)){
			m2_SetU8g(&self->u8g, m2_u8g_box_icon);
	
			m2_CheckKey();
			m2_HandleKey();  
			u8g_FirstPage(&self->u8g);
      do{
        m2_Draw();
      } while( u8g_NextPage(&self->u8g) );
			self->time = timestamp_from_now_us(1000000/25); 
		}
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

static void _m2_fb_set_pixel8(struct m2_fb *self, u8g_dev_arg_pixel_t *arg_pixel){
	register uint8_t pixel = arg_pixel->pixel;
	do {
		if ( pixel & 128 ) {
			//printf("8pix %d %d %d\n", arg_pixel->x, arg_pixel->y, arg_pixel->color); 
			fbuf_set_pixel(self->fbuf, arg_pixel->x, arg_pixel->y, arg_pixel->color); 
			//u8g_pb8v1_SetPixel(b, arg_pixel);
		}
		switch( arg_pixel->dir ) {
			case 0: arg_pixel->x++; break;
			case 1: arg_pixel->y++; break;
			case 2: arg_pixel->x--; break;
			case 3: arg_pixel->y--; break;
		}
		pixel <<= 1;
	} while( pixel != 0  );
}


static uint8_t _m2_gh_u8g_fn(m2_gfx_arg_p  arg)
{
	switch(arg->msg) {
		case M2_GFX_MSG_DRAW_NORMAL_NO_FOCUS:
			if ( (arg->font & 4) != 0 )
			{
				m2_u8g_draw_frame(arg->x, arg->y, arg->w, arg->h + 1);
			}
			//m2_u8g_draw_frame(arg->x, arg->y, arg->w, arg->h);
			
			m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
			if ( m2_u8g_dev_variables.m2_gh_u8g_invert_at_depth < m2_u8g_dev_variables.m2_gh_u8g_current_depth )
			{
				m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
			}
			break;
		case M2_GFX_MSG_DRAW_NORMAL_PARENT_FOCUS:
			break;
		case M2_GFX_MSG_DRAW_NORMAL_FOCUS:
			m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
			m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h + 1);
			m2_u8g_draw_frame(arg->x, arg->y, arg->w, arg->h + 1);
			
			/* in all cases, the text below needs to be inverted */
			m2_u8g_dev_variables.m2_gh_u8g_invert_at_depth = m2_u8g_dev_variables.m2_gh_u8g_current_depth;
			// printf("invert %d, width %d x:%d y:%d\n", m2_gh_u8g_invert_at_depth, arg->w, arg->x, arg->y);
			break;
		case M2_GFX_MSG_DRAW_SMALL_FOCUS:
			m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_bg_text_color;
			m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h);
			break;
		case M2_GFX_MSG_DRAW_GO_UP:
			/* does not work because of missing xor...
				 also: can not be fixed with FRAME_DRAW_AT_END
			 m2_u8g_current_text_color = m2_u8g_bg_text_color; */
			m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h/2);
			break;
		case M2_GFX_MSG_GET_NORMAL_BORDER_HEIGHT:
			return 1;
		case M2_GFX_MSG_GET_NORMAL_BORDER_WIDTH:
			return 1; 
		case M2_GFX_MSG_GET_READONLY_BORDER_X_OFFSET: 
		case M2_GFX_MSG_GET_NORMAL_BORDER_X_OFFSET:
			return 1; 
		case M2_GFX_MSG_GET_READONLY_BORDER_Y_OFFSET: 
		case M2_GFX_MSG_GET_NORMAL_BORDER_Y_OFFSET:
			return 1;
		case M2_GFX_MSG_GET_LIST_OVERLAP_HEIGHT:
			return 0;
		case M2_GFX_MSG_GET_LIST_OVERLAP_WIDTH:
			return 0;
		case M2_GFX_MSG_IS_FRAME_DRAW_AT_END:
			return 0; /* focus (highlight) is drawn first, then the text string */
	}

	return m2_gh_u8g_base(arg);
}


static uint8_t _m2_fb_u8g_dev_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
	(void)dev; 
	
	struct m2_fb *self = container_of(u8g, struct m2_fb, u8g); 
	
	switch(msg)
	{
		case U8G_DEV_MSG_SET_8PIXEL:
			_m2_fb_set_pixel8(self, (u8g_dev_arg_pixel_t *)arg); 
			break;
		case U8G_DEV_MSG_SET_PIXEL: {
			u8g_dev_arg_pixel_t *pix = (u8g_dev_arg_pixel_t *)arg; 
			fbuf_set_pixel(self->fbuf, pix->x, pix->y, pix->color); 
		} break;
		case U8G_DEV_MSG_INIT:
			break;
		case U8G_DEV_MSG_STOP:
			break;
		case U8G_DEV_MSG_PAGE_FIRST:
			fbuf_clear(self->fbuf);
			break;
		case U8G_DEV_MSG_PAGE_NEXT: {
			return 0; 
		} break;
		case U8G_DEV_MSG_GET_PAGE_BOX: {
			//printf("getpagebox\n"); 
			u8g_box_t *box = (u8g_box_t *)arg; 
			box->x0 = 0;
			box->y0 = 0;
			box->x1 = self->width - 1;
			box->y1 = self->height;
			//u8g_pb_GetPageBox(pb, (u8g_box_t *)arg);
		} break;
		case U8G_DEV_MSG_GET_WIDTH: {
			*((u8g_uint_t *)arg) = self->width; 
		} break;
		case U8G_DEV_MSG_GET_HEIGHT: {
			*((u8g_uint_t *)arg) = self->height; 
		} break;
		case U8G_DEV_MSG_SET_COLOR_ENTRY:
			break;
		case U8G_DEV_MSG_SET_XY_CB:
			break;
		case U8G_DEV_MSG_GET_MODE:
			return U8G_MODE_BW;
	}
	return 1;
}

static u8g_dev_t u8g_dev_libk_framebuffer = {
	.dev_fn = _m2_fb_u8g_dev_fn, 
	.dev_mem = 0, 
	.com_fn = u8g_com_null_fn
}; 

void m2_fb_init(fbuf_dev_t _fb, m2_rom_void_p element){
	struct m2_fb *self = &_m2fb; 
	
	self->fbuf = _fb; 
	fbuf_get_size(self->fbuf, &self->width, &self->height); 

	u8g_Init(&self->u8g, &u8g_dev_libk_framebuffer); 
  
	m2_Init(element, _m2_fb_es, m2_eh_6bs, _m2_gh_u8g_fn);
	
	m2_SetU8g(&self->u8g, m2_u8g_box_icon);
	m2_SetFont(0, (const void *)u8g_font_5x7);
	
	cbuf_init(&self->key_buffer, self->key_buffer_data, sizeof(self->key_buffer_data)); 
	
	libk_create_thread(&self->thread, _m2_fb_thread, "m2_fb"); 
}


void m2_fb_put_key(uint8_t key){
	struct m2_fb *self = &_m2fb; 
	
	cbuf_put(&self->key_buffer, key); 
}
