/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <tty/vt100.h>
#include <tty/m2_tty.h>

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


/*======================================================================*/
/* extern ref to top element */
M2_EXTERN_ALIGN(top_el_tlsm);

/* dummy button callback */
void fn_dummy(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  /* accept selection */
}

/*======================================================================*/
/* radio buttons */

uint8_t rb_select_color = 0;


M2_LABEL(el_rb_label1, NULL, "red");
M2_RADIO(el_rb_radio1, "v0", &rb_select_color);

M2_LABEL(el_rb_label2, NULL, "green");
M2_RADIO(el_rb_radio2, "v1", &rb_select_color);

M2_LABEL(el_rb_label3, NULL, "blue");
M2_RADIO(el_rb_radio3, "v2", &rb_select_color);

M2_ROOT(el_rb_goto_top, NULL, " top menu ", &top_el_tlsm);


M2_LIST(list_rb) = { 
    &el_rb_label1, &el_rb_radio1, 
    &el_rb_label2, &el_rb_radio2,  
    &el_rb_label3, &el_rb_radio3, 
    &el_rb_goto_top
};
M2_GRIDLIST(el_top_rb, "c2",list_rb);


/*======================================================================*/
/* text entry */
char text_str[5] = "(AgI";

M2_LABEL(el_te_l1, NULL, "TE a0:");
M2_TEXT(el_te1, "a0", text_str, 4);
M2_LABEL(el_te_l2, NULL, "TE a1:");
M2_TEXT(el_te2, "a1", text_str, 4);
M2_ROOT(el_te_goto_top, NULL, "top menu", &top_el_tlsm);

M2_LIST(list_te) = { 
    &el_te_l1, &el_te1, 
    &el_te_l2, &el_te2,  
    &el_te_goto_top 
};
M2_GRIDLIST(el_top_te, "c2", list_te);


/*======================================================================*/
/* button examples */


M2_BUTTON(el_btn_highlight, "w125f4", "highlight", fn_dummy);
M2_BUTTON(el_btn_normal, "w128f0", "normal", fn_dummy);

M2_ROOT(el_btn_goto_top, "w128", "top menu", &top_el_tlsm);

M2_LIST(list_btn) = { 
    &el_btn_highlight, 
    &el_btn_normal, 
    &el_btn_goto_top
};

M2_VLIST(el_top_btn, NULL, list_btn);


/*======================================================================*/
/* combo examples */

uint8_t select_color = 0;
uint8_t select_priority = 0;

void fn_ok(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  /* accept selection */
}

void fn_cancel(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  /* discard selection */
  m2_SetRoot(&top_el_tlsm);
}

const char *fn_idx_to_color(uint8_t idx)
{
  if ( idx == 0 )
    return "red";
  else if (idx == 1 )
    return "green";
  return "blue";
}

const char *fn_idx_to_priority(uint8_t idx)
{
  switch(idx)
  {
    case 0: return "lowest";
    case 1: return "low";
    case 2: return "medium";
    case 3: return "high";
    case 4: return "highest";
  }
  return "";
}


M2_LABEL(el_label1, NULL, "Color:");
M2_COMBO(el_combo1, NULL, &select_color, 3, fn_idx_to_color);

M2_LABEL(el_label2, NULL, "Priority: ");
M2_COMBO(el_combo2, "v1", &select_priority, 5, fn_idx_to_priority);

M2_BUTTON(el_cancel, "f4", "cancel", fn_cancel);
M2_BUTTON(el_ok, "f4", "ok", fn_ok);

M2_LIST(list_combo) = { 
    &el_label1, &el_combo1, 
    &el_label2, &el_combo2,  
    &el_cancel, &el_ok 
};
M2_GRIDLIST(el_top_combo, "c2", list_combo);

/*======================================================================*/
/* save as (reuse buttons from combo) */
char save_as_str[13] = "filename.txt";

M2_LABEL(el_sa_label, NULL, "Save as:");
M2_TEXT(el_sa_text, "a0", save_as_str, 12);

M2_LIST(list_ok_cancel) = {     &el_cancel, &el_ok };
M2_HLIST(el_ok_cancel, NULL, list_ok_cancel);

M2_LIST(list_sa) = {     &el_sa_label, &el_sa_text, &el_ok_cancel };
M2_VLIST(el_sa, NULL, list_sa);
 
M2_ALIGN(top_el_sa, "-1|1W64H64", &el_sa);

/*======================================================================*/
/* speed mph kmh */
uint8_t speed = 108;
M2_U8NUM(el_num_speed, "f2r1", 0, 255, &speed);

M2_LABEL(el_speed_unit, NULL, "mph");

M2_LIST(list_speed_num) = { &el_num_speed, &el_speed_unit };
M2_HLIST(el_speed_num, NULL, list_speed_num);

M2_SPACE(el_speed_space, "h10w1");


M2_LIST(list_speed) = { &el_speed_num, &el_speed_space, &el_ok_cancel };
M2_VLIST(el_speed, NULL, list_speed);
 
M2_ALIGN(top_el_speed, "-1|1W64H64", &el_speed);

/*======================================================================*/
/* number entry */

uint8_t u8num = 0;
int8_t s8num = 120;
uint32_t u32num = 0;

void fn_num_zero(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  u8num = 0;
  s8num = 0;
  u32num = 0;
}

M2_LABEL(el_num_label1, NULL, "U8:");
M2_U8NUM(el_num_1, "q2", 0, 255, &u8num);

M2_LABEL(el_num_label2, NULL, "S8:");
M2_S8NUM(el_num_2, "+1c3", -128, 127, &s8num);

M2_LABEL(el_num_label3, NULL, "U32:");
M2_U32NUM(el_num_3, "c5", &u32num);

M2_BUTTON(el_num_zero, "f4q1", "1:zero", fn_num_zero);
M2_ROOT(el_num_goto_top, "f4", "back", &top_el_tlsm);

M2_LIST(num_list) = { 
    &el_num_label1, &el_num_1, 
    &el_num_label2, &el_num_2,  
    &el_num_label3, &el_num_3,  
    &el_num_zero, &el_num_goto_top
};
M2_GRIDLIST(el_num_menu, "c2", num_list);

/*======================================================================*/
/* number entry with TSK buttons*/


M2_TSK(el_tsnum_select, "", "select", M2_KEY_SELECT);
M2_TSK(el_tsnum_next, "", "next", M2_KEY_NEXT);
M2_TSK(el_tsnum_prev, "", "prev", M2_KEY_PREV);

M2_LIST(tsnum_list) = { 
    &el_tsnum_select, &el_num_label1, &el_num_1, 
    &el_tsnum_next, &el_num_label2, &el_num_2,  
    &el_tsnum_prev, &el_num_zero, &el_num_goto_top
};
M2_GRIDLIST(el_tsnum_menu, "c3", tsnum_list);


/*======================================================================*/
/* single focus mode */

M2_ROOT(el_ts_mnu1_sel, "t1", "Menu 1 selected", &top_el_tlsm);
M2_ALIGN(top_el_ts_mnu1_sel, "-1|1W64H64", &el_ts_mnu1_sel);
M2_ROOT(el_ts_mnu2_sel, "t1", "Menu 2 selected", &top_el_tlsm);
M2_ALIGN(top_el_ts_mnu2_sel, "-1|1W64H64", &el_ts_mnu2_sel);
M2_ROOT(el_ts_mnu3_sel, "t1", "Menu 3 selected", &top_el_tlsm);
M2_ALIGN(top_el_ts_mnu3_sel, "-1|1W64H64", &el_ts_mnu3_sel);


M2_ROOT(el_ts_mnu1, "t1w60f8", "menu 1", &top_el_ts_mnu1_sel);
M2_ROOT(el_ts_mnu2, "t1w60f8", "menu 2", &top_el_ts_mnu2_sel);
M2_ROOT(el_ts_mnu3, "t1w60f8", "menu 3", &top_el_ts_mnu3_sel);

M2_LIST(list_ts_mnu) = { 
    &el_ts_mnu1, 
    &el_ts_mnu2, 
    &el_ts_mnu3
};

M2_VLIST(el_ts_mnu_vlist, NULL, list_ts_mnu);
M2_ALIGN(top_el_ts_mnu, "-1|1W64H64", &el_ts_mnu_vlist);

/*======================================================================*/
/* double focus mode */
/* 
  number entry with TSK buttons 

  Num: 000000
  . up .
  left ok right
  . down .
*/

uint32_t u32val = 0;
M2_LABEL(el_tsk_num_label, NULL, "U32:");
M2_U32NUM(el_tsk_num_u32, "a1c4.1", &u32val);

M2_TSK(el_tsk_up, "f1", " \xdd ", M2_KEY_DATA_UP);		// data up
M2_TSK(el_tsk_down, "f1", " \xdf ", M2_KEY_DATA_DOWN);		// data down
M2_TSK(el_tsk_left, "f1", " \xdc ", M2_KEY_PREV);		// left
M2_TSK(el_tsk_right, "f1", " \xde ", M2_KEY_NEXT);		// right
M2_ROOT(el_tsk_enter, "f1t1r1", " \xbf ", &top_el_tlsm);		// enter

M2_LIST(tsk_list) = { 
    &el_tsk_num_label, &m2_null_element, &el_tsk_up,    &m2_null_element, 
    &el_tsk_num_u32,  &el_tsk_left,            &el_tsk_enter, &el_tsk_right, 
    &m2_null_element, &m2_null_element, &el_tsk_down, &m2_null_element, 
};
M2_GRIDLIST(el_tsk_num_menu, "c4", tsk_list);
M2_ALIGN(top_el_tsk_num_menu, "-1|1W64H64", &el_tsk_num_menu);

/*======================================================================*/
/* double focus mode */
/* 
  number entry with XBMTSK buttons 

  Num: 000000
  . up .
  left ok right
  . down .
*/

/*
static unsigned char down_bits[] U8G_PROGMEM = {
   0x00, 0x00, 0xf8, 0x1f, 0x04, 0x20, 0x82, 0x41, 0x82, 0x41, 0x82, 0x41,
   0x82, 0x41, 0x82, 0x41, 0x82, 0x41, 0xf2, 0x4f, 0xe2, 0x47, 0xc2, 0x43,
   0x82, 0x41, 0x04, 0x20, 0xf8, 0x1f, 0x00, 0x00};
static unsigned char enter_bits[] U8G_PROGMEM  = {
   0x00, 0x00, 0xf8, 0x1f, 0x04, 0x20, 0x02, 0x40, 0x02, 0x40, 0x02, 0x4c,
   0x02, 0x4c, 0x22, 0x4c, 0x32, 0x4c, 0xfa, 0x4f, 0xfa, 0x4f, 0x32, 0x40,
   0x22, 0x40, 0x04, 0x20, 0xf8, 0x1f, 0x00, 0x00};
static unsigned char left_bits[] U8G_PROGMEM  = {
   0x00, 0x00, 0xf8, 0x1f, 0x04, 0x20, 0x02, 0x40, 0x42, 0x40, 0x62, 0x40,
   0x72, 0x40, 0xfa, 0x5f, 0xfa, 0x5f, 0x72, 0x40, 0x62, 0x40, 0x42, 0x40,
   0x02, 0x40, 0x04, 0x20, 0xf8, 0x1f, 0x00, 0x00};
static unsigned char right_bits[] U8G_PROGMEM  = {
   0x00, 0x00, 0xf8, 0x1f, 0x04, 0x20, 0x02, 0x40, 0x02, 0x42, 0x02, 0x46,
   0x02, 0x4e, 0xfa, 0x5f, 0xfa, 0x5f, 0x02, 0x4e, 0x02, 0x46, 0x02, 0x42,
   0x02, 0x40, 0x04, 0x20, 0xf8, 0x1f, 0x00, 0x00};
static unsigned char up_bits[] U8G_PROGMEM  = {
   0x00, 0x00, 0xf8, 0x1f, 0x04, 0x20, 0x82, 0x41, 0xc2, 0x43, 0xe2, 0x47,
   0xf2, 0x4f, 0x82, 0x41, 0x82, 0x41, 0x82, 0x41, 0x82, 0x41, 0x82, 0x41,
   0x82, 0x41, 0x04, 0x20, 0xf8, 0x1f, 0x00, 0x00};


uint32_t xu32val = 0;
M2_LABEL(el_xtsk_num_label, NULL, "U32:");
M2_U32NUM(el_xtsk_num_u32, "a1c4.1", &xu32val);


M2_XBMTSKP(el_xtsk_up, "f1w16h16r1", 16, 16, up_bits, M2_KEY_DATA_UP);		// data up
M2_XBMTSKP(el_xtsk_down, "f1w16h16", 16, 16, down_bits, M2_KEY_DATA_DOWN);		// data down
M2_XBMTSKP(el_xtsk_left, "f1w16h16", 16, 16, left_bits, M2_KEY_PREV);		// left
M2_XBMTSKP(el_xtsk_right, "f1w16h16", 16, 16, right_bits, M2_KEY_NEXT);		// right
M2_XBMROOTP(el_xtsk_enter, "f1t1r1w16h16", 16, 16, enter_bits, &top_el_tlsm);		// enter



M2_LIST(xtsk_list) = { 
    &el_xtsk_num_label, &m2_null_element, &el_xtsk_up,    &m2_null_element, 
    &el_xtsk_num_u32,  &el_xtsk_left,            &el_xtsk_enter, &el_xtsk_right, 
    &m2_null_element, &m2_null_element, &el_xtsk_down, &m2_null_element, 
};
M2_GRIDLIST(el_xtsk_num_menu, "c4", xtsk_list);
M2_ALIGN(top_el_xtsk_num_menu, "-1|1W64H64", &el_xtsk_num_menu);

M2_LIST(xtsk2_list) = { &el_xtsk_num_label, &el_xtsk_num_u32 };
M2_GRIDLIST(el_xtsk2_num_menu, "c2", xtsk2_list);

M2_BOX(xtsk_box, "w128h2");

M2_LIST(xtsk3_list) = { &el_xtsk_left,  &el_xtsk_up, &el_xtsk_enter, &el_xtsk_down, &el_xtsk_right, };
M2_GRIDLIST(el_xtsk3_num_menu, "c5", xtsk3_list);

M2_LIST(xtsk_vlist) = { &el_xtsk2_num_menu, &xtsk_box, &el_xtsk3_num_menu };
M2_VLIST(el_xtsk_vlist, "", xtsk_vlist);
M2_ALIGN(top_el_xtsk_num_box_menu, "-1|1W64H64", &el_xtsk_vlist);

*/

/*======================================================================*/
/* single focus mode */
/* 
  number entry with TSK buttons 

  Num: left 00 right ok
*/

uint8_t u8val = 0;
M2_LABEL(el_ts_u8_label, NULL, "U8: ");

void fn_ts_dec(m2_el_fnarg_p fnarg) { (void)fnarg; u8val--; }
M2_BUTTON(el_ts_dec, "f1r1t1", " \xdc ", fn_ts_dec);

M2_U8NUM(el_ts_u8_num, "r1c3", 0, 255, &u8val);

void fn_ts_inc(m2_el_fnarg_p fnarg) { (void)fnarg; u8val++; }
M2_BUTTON(el_ts_inc, "f1r1t1", " \xde ", fn_ts_inc);


M2_ROOT(el_ts_enter, "f1r1t1", " \xbf ", &top_el_tlsm);		// enter



M2_LIST(ts_u8_list) = { 
    &el_ts_u8_label, &el_ts_dec, &el_ts_u8_num, &el_ts_inc, &el_ts_enter
};
M2_HLIST(el_ts_u8_menu, "", ts_u8_list);
M2_ALIGN(top_el_ts_u8_menu, "-1|1W64H64", &el_ts_u8_menu);


/*======================================================================*/



/*======================================================================*/
/* gfx handler selection: gfx */

/*
const char *el_gfx_strlist_cb(uint8_t idx, uint8_t msg) {
  const char *s = "";
  if  ( idx == 0 )
    s = "fb";
  else if ( idx == 1 )
    s = "bf";
  else if ( idx == 2 )
    s = "bfs";
  else if ( idx == 3 )
    s = "ffs";

  if (msg == M2_STRLIST_MSG_GET_STR) {
   
  } 
  else if ( msg == M2_STRLIST_MSG_SELECT ) {
    if ( idx == 0 ) {
      m2_SetGraphicsHandler(m2_gh_u8g_fb);
    }
    else if ( idx == 1 ) {
      m2_SetGraphicsHandler(m2_gh_u8g_bf);
    }
    else if ( idx == 2 ) {
      m2_SetGraphicsHandler(m2_gh_u8g_bfs);
    }
    else if ( idx == 3 ) {
      m2_SetGraphicsHandler(m2_gh_u8g_ffs);
    }
    m2_SetRoot(&top_el_tlsm);
  }
  return s;
}

uint8_t el_gfx_first = 0;
uint8_t el_gfx_cnt = 4;

M2_STRLIST(el_gfx_strlist, "l3W56", &el_gfx_first, &el_gfx_cnt, el_gfx_strlist_cb);
M2_SPACE(el_gfx_space, "W1h1");
M2_VSB(el_gfx_vsb, "l3W4r1", &el_gfx_first, &el_gfx_cnt);
M2_LIST(list_gfx_strlist) = { &el_gfx_strlist, &el_gfx_space, &el_gfx_vsb };
M2_HLIST(el_gfx_hlist, NULL, list_gfx_strlist);

M2_ALIGN(top_el_gfx, "-1|1W64H64", &el_gfx_hlist);

*/
/*======================================================================*/
/* font selection:fsel */
/*
const char *el_fsel_strlist_cb(uint8_t idx, uint8_t msg) {
  const char *s = "";

  if ( idx == 0 ) {
    s = "4x6";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_4x6);
      m2_SetU8gToggleFontIcon(u8g_font_6x12_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_6x12_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 1 ) {
    s = "5x8";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_5x8);
      m2_SetU8gToggleFontIcon(u8g_font_6x12_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_6x12_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 2 ) {
    s = "6x12";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_6x12);
      m2_SetU8gToggleFontIcon(u8g_font_6x12_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_6x12_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 3 ) {
    s = "6x13";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_6x13);
      m2_SetU8gToggleFontIcon(u8g_font_6x13_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_6x13_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 4 ) {
    s = "7x13";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_7x13);
      m2_SetU8gToggleFontIcon(u8g_font_7x13_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_7x13_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 5 ) {
    s = "8x13";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_8x13);
      m2_SetU8gToggleFontIcon(u8g_font_8x13_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_8x13_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 6 ) {
    s = "tpss";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_tpss);
      m2_SetU8gToggleFontIcon(u8g_font_7x13_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_7x13_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 7 ) {
    s = "cu12";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_cu12);
      m2_SetU8gToggleFontIcon(u8g_font_cu12_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_cu12_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 8 ) {
    s = "unifont";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_unifont);
      m2_SetU8gToggleFontIcon(u8g_font_unifont_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_unifont_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  else if ( idx == 9 ) {
    s = "9x15";
    if ( msg == M2_STRLIST_MSG_SELECT ) {
      m2_SetFont(0, (const void *)u8g_font_9x15);
      m2_SetU8gToggleFontIcon(u8g_font_9x15_75r, active_encoding, inactive_encoding);
      m2_SetU8gRadioFontIcon(u8g_font_9x15_75r, active_encoding, inactive_encoding);
      m2_SetRoot(&top_el_tlsm);
    }
  }
  return s;
}

uint8_t el_fsel_first = 0;
uint8_t el_fsel_cnt = 10;

M2_STRLIST(el_fsel_strlist, "l3W56", &el_fsel_first, &el_fsel_cnt, el_fsel_strlist_cb);
M2_SPACE(el_fsel_space, "W1h1");
M2_VSB(el_fsel_vsb, "l3W4r1", &el_fsel_first, &el_fsel_cnt);
M2_LIST(list_fsel_strlist) = { &el_fsel_strlist, &el_fsel_space, &el_fsel_vsb };
M2_HLIST(el_fsel_hlist, NULL, list_fsel_strlist);

M2_ALIGN(top_el_fsel, "-1|1W64H64", &el_fsel_hlist);
*/
/*======================================================================*/
/* Edit x width properties */
/*
void fn_xwp_zero(m2_el_fnarg_p fnarg) {
  m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size = 0;
  m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size = 0;
}

M2_LABEL(el_xwp_label1, NULL, "Add to Frame:");
M2_U8NUM(el_xwp_1, "c1", 0, 4, &m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size);

M2_LABEL(el_xwp_label2, NULL, "Add to Text:");
M2_U8NUM(el_xwp_2, "c1", 0, 4, &m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size);

M2_ROOT(el_xwp_goto_top, "f4", "Goto Top Menu", &top_el_tlsm);
M2_BUTTON(el_xwp_zero, "f4", "Zero", fn_xwp_zero);

M2_LIST(xwp_list) = { 
    &el_xwp_label1, &el_xwp_1, 
    &el_xwp_label2, &el_xwp_2,  
    &el_xwp_goto_top, &el_xwp_zero
};
M2_GRIDLIST(el_xwp_menu, "c2", xwp_list);
M2_ALIGN(top_el_xwp, "-1|1W64H64", &el_xwp_menu);
*/
/*======================================================================*/
/* hello world label */
M2_LABEL(hello_world_label, NULL, "Hello World");

/*======================================================================*/
/* Edit a long int number */
uint32_t number = 1234;
M2_U32NUM(el_u32num, "a1c4", &number);

/*=========================================================================*/
/* edit date dialog */

uint8_t dt_day = 1;
uint8_t dt_month = 1;
uint8_t dt_year = 12;

void dt_ok_fn(m2_el_fnarg_p fnarg)  {
	(void)fnarg; 
  m2_SetRoot(&top_el_tlsm);
}

M2_U8NUM(el_dt_day, "c2", 1,31,&dt_day);
M2_LABEL(el_dt_sep1, "b1", ".");
M2_U8NUM(el_dt_month, "c2", 1,12,&dt_month);
M2_LABEL(el_dt_sep2, "b0", ".");
M2_U8NUM(el_dt_year, "c2", 0,99,&dt_year);

M2_LIST(list_date) = { &el_dt_day, &el_dt_sep1, &el_dt_month, &el_dt_sep2, &el_dt_year };
M2_HLIST(el_date, NULL, list_date);

M2_ROOT(el_dt_cancel, NULL, "cancel", &top_el_tlsm);
M2_BUTTON(el_dt_ok, NULL, "ok", dt_ok_fn);
M2_LIST(list_dt_buttons) = {&el_dt_cancel, &el_dt_ok };
M2_HLIST(el_dt_buttons, NULL, list_dt_buttons);

M2_LIST(list_dt) = {&el_date, &el_dt_buttons };
M2_VLIST(el_top_dt, NULL, list_dt);


/*======================================================================*/
/* StrList example */
const char *selected = "Nothing";
const char *el_strlist_getstr(uint8_t idx, uint8_t msg) {
  const char *s = "";
  if  ( idx == 0 )
    s = "Apple";
  else if ( idx == 1 )
    s = "Banana";
  else if ( idx == 2 )
    s = "Peach";
  else if ( idx == 3 )
    s = "Pumpkin";
  else if ( idx == 4 )
    s = "Corn";
  else if ( idx == 5 )
    s = "[Main Menu]";
  if (msg == M2_STRLIST_MSG_GET_STR) {
    /* nothing else todo, return the correct string */
  } else if ( msg == M2_STRLIST_MSG_SELECT ) {
    if ( idx == 5 ) {
      m2_SetRoot(&top_el_tlsm);
    } 
    else {
      selected = s;    
    }
  }
  return s;
}

uint8_t el_strlist_first = 0;
uint8_t el_strlist_cnt = 6;

M2_STRLIST(el_strlist, "l2w90", &el_strlist_first, &el_strlist_cnt, el_strlist_getstr);
M2_SPACE(el_strlist_space, "w1h1");
M2_VSB(el_strlist_vsb, "l2w5r1", &el_strlist_first, &el_strlist_cnt);
M2_LIST(list_strlist) = { &el_strlist, &el_strlist_space, &el_strlist_vsb };
M2_HLIST(el_strlist_hlist, NULL, list_strlist);

M2_LABEL(el_strlist_label,NULL, "Selected:");
M2_LABELPTR(el_strlist_labelptr,NULL, &selected);
M2_LIST(list_strlist_label) = { &el_strlist_label, &el_strlist_labelptr };
M2_HLIST(el_strlist_label_hlist, NULL, list_strlist_label);

M2_LIST(strlist_list) = { &el_strlist_hlist, &el_strlist_label_hlist };
M2_VLIST(el_strlist_vlist, NULL, strlist_list);
M2_ALIGN(top_el_strlist, "-1|1W64H64", &el_strlist_vlist);


/*======================================================================*/
/* multi selection */

#define MULTI_SELECT_CNT 3
const char *multi_select_strings[MULTI_SELECT_CNT] = { "red", "green", "blue" };
uint8_t multi_select_status[MULTI_SELECT_CNT] = { 0, 0, 0};

uint8_t el_muse_first = 0;
uint8_t el_muse_cnt = MULTI_SELECT_CNT;

const char *el_muse_strlist_cb(uint8_t idx, uint8_t msg) {
  const char *s = "";
  if ( msg == M2_STRLIST_MSG_SELECT ) {
    if ( multi_select_status[idx] == 0 ) {
      multi_select_status[idx] = 1;
    }
    else {
      multi_select_status[idx] = 0;
    }
  }
  if ( msg == M2_STRLIST_MSG_GET_STR ) {
    s = multi_select_strings[idx];
  }
  if ( msg == M2_STRLIST_MSG_GET_EXTENDED_STR ) {
    if ( multi_select_status[idx] == 0 ) {
      s = " ";
    }
    else {
      s = "*";
    }
  }
  return s;  
}

M2_STRLIST(el_muse_strlist, "l3F0E10W46", &el_muse_first, &el_muse_cnt, el_muse_strlist_cb);
M2_ROOT(el_muse_goto_top, "f4", "Goto Top Menu", &top_el_tlsm);

M2_LIST(muse_list) = { 
    &el_muse_strlist, 
    &el_muse_goto_top,  
};
M2_VLIST(el_muse_vlist, "c2", muse_list);
M2_ALIGN(top_el_muse, "-1|1W64H64", &el_muse_vlist);

/*======================================================================*/
/* Show selected file */

const char *fs_show_file_label_cb(m2_rom_void_p element)
{
	(void)element; 
  static const char *filename = "-- none --"; 
  return filename; 
}

M2_LABEL(el_show_file_label, NULL, "Selected file:");
M2_LABELFN(el_show_filename, NULL, fs_show_file_label_cb);
M2_ROOT(el_show_file_ok, NULL, "ok", &top_el_tlsm);
M2_LIST(list_show_file) = { &el_show_file_label, &el_show_filename, &el_show_file_ok };
M2_VLIST(el_show_file_Vlist, NULL, list_show_file);
M2_ALIGN(top_el_show_file, "-1|1W64H64", &el_show_file_Vlist);

/* File selection dialog */
/*
#define FS_EXTRA_MENUES 1

uint8_t fs_m2tk_first = 0;
uint8_t fs_m2tk_cnt = 0;

const char *fs_strlist_getstr(uint8_t idx, uint8_t msg)  {
  if (msg == M2_STRLIST_MSG_GET_STR)  {
    if ( idx == 0 )
      return "..";
    return "--nonmas_GetFilename();
  } else if ( msg == M2_STRLIST_MSG_GET_EXTENDED_STR ) {
    if ( idx == 0 )
      return "a";       
    mas_GetDirEntry(idx - FS_EXTRA_MENUES);
    if ( mas_IsDir() )
      return "A";       
    return "B";        
  } else if ( msg == M2_STRLIST_MSG_SELECT ) {
    if ( idx == 0 ) {
      if ( mas_GetPath()[0] == '\0' )
        m2_SetRoot(&top_el_tlsm);      
      else {
        mas_ChDirUp();
        m2_SetRoot(m2_GetRoot());  
      }
    } else {
      mas_GetDirEntry(idx - FS_EXTRA_MENUES);
      if ( mas_IsDir() ) {
        mas_ChDir(mas_GetFilename());
        m2_SetRoot(m2_GetRoot());  
      } else {
        m2_SetRoot(&top_el_show_file);  
      }
    }
  } else if ( msg == M2_STRLIST_MSG_NEW_DIALOG ) {
    if ( mas_GetDirEntryCnt() < 250-FS_EXTRA_MENUES )
      fs_m2tk_cnt = mas_GetDirEntryCnt()+FS_EXTRA_MENUES;
    else
      fs_m2tk_cnt = 250;
  }
  return NULL;
}

M2_STRLIST(el_fs_strlist, "l5F3e15W49", &fs_m2tk_first, &fs_m2tk_cnt, fs_strlist_getstr);
M2_SPACE(el_fs_space, "W1h1");
M2_VSB(el_fs_strlist_vsb, "l5W4r1", &fs_m2tk_first, &fs_m2tk_cnt);
M2_LIST(list_fs_strlist) = { &el_fs_strlist, &el_fs_space, &el_fs_strlist_vsb };
M2_HLIST(el_top_fs, NULL, list_fs_strlist);

M2_STRLIST(el_fs2_strlist, "l5W57", &fs_m2tk_first, &fs_m2tk_cnt, fs_strlist_getstr);
M2_SPACE(el_fs2_space, "W1h1");
M2_VSB(el_fs2_strlist_vsb, "l5W4r1", &fs_m2tk_first, &fs_m2tk_cnt);
M2_LIST(list_fs2_strlist) = { &el_fs2_strlist, &el_fs2_space, &el_fs2_strlist_vsb };
M2_HLIST(el_top_fs2, NULL, list_fs2_strlist);
*/

/*======================================================================*/
/* menue examples: show several menue types 
  VLIST Menu: M2_ROOT within M2_VLIST
    Advantage: No callback procedure required, automatic menu width calculation
  STRLIST Menu: M2_STRLIST 
    Advantage: Scrollable, simple form has only one element

  Nested Menues:
    VLIST and STRLIST Menus can nest each other.

*/

M2_EXTERN_ALIGN(top_el_select_menu);

/*=== VLIST Sub-Menu ===*/
/* each menu line is defined by a M2_ROOT element */
M2_ROOT(el_vlistsum_m1, NULL, "Sub-Menu 1", &top_el_select_menu);
M2_ROOT(el_vlistsum_m2, NULL, "Sub-Menu 2", &top_el_select_menu);
M2_ROOT(el_vlistsum_m3, NULL, "Sub-Menu 3", &top_el_select_menu);
M2_ROOT(el_vlistsum_m4, NULL, "Sub-Menu 4", &top_el_select_menu);
/* all menu lines are grouped by a vlist element */
M2_LIST(list_vlistsum) = { &el_vlistsum_m1, &el_vlistsum_m2, &el_vlistsum_m3, &el_vlistsum_m4 };
M2_VLIST(el_vlistsum_list, NULL, list_vlistsum);
/* center the menu on the display */
M2_ALIGN(top_el_vlist_submenu, "-1|1W64H64", &el_vlistsum_list);

/*=== VLIST Main-Menu with VLIST Sub-Menues ===*/
/* each menu line is defined by a M2_ROOT element */
/* in this example, all four menues refer to the same submenu: top_el_vlist_submenu */
M2_ROOT(el_vlistmm_m1, NULL, "Menu 1", &top_el_vlist_submenu);
M2_ROOT(el_vlistmm_m2, NULL, "Menu 2", &top_el_vlist_submenu);
M2_ROOT(el_vlistmm_m3, NULL, "Menu 3", &top_el_vlist_submenu);
M2_ROOT(el_vlistmm_m4, NULL, "Menu 4", &top_el_vlist_submenu);

/* all menu lines are grouped by a vlist element */
M2_LIST(list_vlistmm) = { &el_vlistmm_m1, &el_vlistmm_m2, &el_vlistmm_m4, &el_vlistmm_m4 };
M2_VLIST(el_vlistmm_list, NULL, list_vlistmm);

/* center the menu on the display */
M2_ALIGN(top_el_vlist_mainmenu, "-1|1W64H64", &el_vlistmm_list);

/*=== STRLIST Sub-Menu ===*/
/* all STRLIST elements requre the following two variables */
uint8_t el_strlistsum_first = 0;
uint8_t el_strlistsum_cnt = 4;
/* sub menu names can be stored in a string list */
const char * strlistsumdefs[] = { "Sub-Menu 1", "Sub-Menu 2", "Sub-Menu 3", "Sub-Menu 4" };  
/* the STRLIST element always requires a callback procedure */
const char *el_strlistsum_cb(uint8_t idx, uint8_t msg) {
  /* for any value in "idx", jumpo back to the selection menu */
  if ( msg == M2_STRLIST_MSG_SELECT )
    m2_SetRoot(&top_el_select_menu);
  
  /* "idx" contains the selected or requested submenu line (starts with 0) */
  return strlistsumdefs[idx];;
}

M2_STRLIST(el_strlistsum, "l4W64", &el_strlistsum_first, &el_strlistsum_cnt, el_strlistsum_cb);
/* center the STRLIST sub-menu on the display */
M2_ALIGN(top_el_strlist_submenu, "-1|1W64H64", &el_strlistsum);


/*=== STRLIST Main-Menu with STRLIST Sub-Menues ===*/
/* all STRLIST elements requre the following two variables */
uint8_t el_strlistmm_first = 0;
uint8_t el_strlistmm_cnt = 4;
/* sub menu names can be stored in a string list */
const char * strlistmmdefs[] = { "Menu 1", "Menu 2", "Menu 3", "Menu 4" };  
/* the STRLIST element always requires a callback procedure */
const char *el_strlistmm_cb(uint8_t idx, uint8_t msg) {
  /* for any value in "idx", jumpo to the sub-menu */
  if ( msg == M2_STRLIST_MSG_SELECT )
    m2_SetRoot(&top_el_strlist_submenu);
  
  /* "idx" contains the selected or requested submenu line (starts with 0) */
  return strlistmmdefs[idx];;
}

M2_STRLIST(el_strlistmm, "l4W64", &el_strlistmm_first, &el_strlistmm_cnt, el_strlistmm_cb);
/* center the STRLIST sub-menu on the display */
M2_ALIGN(top_el_strlist_mainmenu, "-1|1W64H64", &el_strlistmm);

/*=== Expandable Menu ===*/

m2_menu_entry menu_data[] = 
{
  { "Menu 1", NULL },
  { ". Sub 1-1", &top_el_select_menu },
  { ". Sub 1-2", &top_el_select_menu },
  { "Menu 2", &top_el_select_menu },
  { "Menu 3", NULL },
  { ". Sub 3-1", &top_el_select_menu },
  { ". Sub 3-2", &top_el_select_menu },
  { "Menu 4", &top_el_select_menu },
  { "Menu 5", NULL },
  { ". Sub 5-1", &top_el_select_menu },
  { ". Sub 5-2", &top_el_select_menu },
  { ". Sub 5-3", &top_el_select_menu },
  { NULL, NULL },
};



//M2_STRLIST(el_exme_strlist, "l4e15W47", &el_exme_first, &el_exme_cnt, el_exme_strlist_cb);
/* this has been removed, use M2_2LMENU
M2_STRLIST(el_exme_strlist, "l4e15W47", &m2_strlist_menu_first, &m2_strlist_menu_cnt, m2_strlist_menu_cb);

M2_SPACE(el_exme_space, "W1h1");
M2_VSB(el_exme_vsb, "l4W4r1", &m2_strlist_menu_first, &m2_strlist_menu_cnt);
M2_LIST(list_exme_strlist) = { &el_exme_strlist, &el_exme_space, &el_exme_vsb };
M2_HLIST(el_exme_hlist, NULL, list_exme_strlist);
M2_ALIGN(top_el_expandable_menu, "-1|1W64H64", &el_exme_hlist);
*/

/*=== Expandable Menu 2 ===*/

uint8_t el_2lme_first = 0;
uint8_t el_2lme_cnt = 3;

/* for m2icon fonts, 65: closed folder, 102: open folder */
M2_2LMENU(el_2lme_strlist, "l4e15F3W47", &el_2lme_first, &el_2lme_cnt, menu_data, 65, 102, '\0');
M2_SPACE(el_2lme_space, "W1h1");
M2_VSB(el_2lme_vsb, "l4W4r1", &el_2lme_first, &el_2lme_cnt);
M2_LIST(list_2lme_strlist) = { &el_2lme_strlist, &el_2lme_space, &el_2lme_vsb };
M2_HLIST(el_2lme_hlist, NULL, list_2lme_strlist);
M2_ALIGN(top_el_2l_menu, "-1|1W64H64", &el_2lme_hlist);

/*=== Menu Selection ===*/

uint8_t el_seme_first = 0;
uint8_t el_seme_cnt = 3;
const char *el_seme_strlist_cb(uint8_t idx, uint8_t msg) {
  const char *s = "";
  if ( idx == 0 ) {
    s = "VLIST->VLIST";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&top_el_vlist_mainmenu);
  }
  else if ( idx == 1 ) {
    s = "STRLIST->STRLIST";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&top_el_strlist_mainmenu);
  }
  /*
  else if ( idx == 2 ) {
    s = "Expandable Menu";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetStrlistMenuData(exmedef, '+', '-', ' ');
      m2_SetRoot(&top_el_expandable_menu);
    }
  }
  */
  else if ( idx == 2 ) {
    s = "2L Menu";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      //el_exme_update_cnt();
      m2_SetRoot(&top_el_2l_menu);
    }
  }

  
  return s;
}


M2_STRLIST(el_seme_strlist, "l3W56", &el_seme_first, &el_seme_cnt, el_seme_strlist_cb);
M2_SPACE(el_seme_space, "W1h1");
M2_VSB(el_seme_vsb, "l3W4r1", &el_seme_first, &el_seme_cnt);
M2_LIST(list_seme_strlist) = { &el_seme_strlist, &el_seme_space, &el_seme_vsb };
M2_HLIST(el_seme_hlist, NULL, list_seme_strlist);
M2_ALIGN(top_el_select_menu, "-1|1W64H64", &el_seme_hlist);


/*======================================================================*/

M2_EXTERN_ALIGN(el_top_controller_menu);

// ====  Light Dialog Window ====

// the following variables contain the input values from the user
uint8_t light_timer = 0;
uint8_t light_on = 0;

// definition of the dialog window starts here

M2_LABEL(el_light_on_label, NULL, "Light On: ");
M2_TOGGLE(el_light_on_toggle, NULL, &light_on);

M2_LABEL(el_light_timer_label, NULL, "Timer: ");
M2_U8NUM(el_light_timer_u8, "c2", 0, 99, &light_timer);

void light_dialog_ok(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  /* do something with the values */
  /* ... */
  
  /* go back to main menu */
  m2_SetRoot(&el_top_controller_menu);  
}

M2_BUTTON(el_light_ok, "f4", " ok ", light_dialog_ok);

M2_LIST(list_light_dialog) = { 
    &el_light_on_label, &el_light_on_toggle, 
    &el_light_timer_label, &el_light_timer_u8,  
    &el_light_ok
};
M2_GRIDLIST(el_light_grid, "c2", list_light_dialog);

M2_ALIGN(el_top_light, "-1|1W64H64", &el_light_grid);

// ====  Main Menu ====

M2_ROOT(list_controller_light, NULL, "Light", &el_top_light);
M2_ROOT(list_controller_power, NULL, "Power", &el_top_light);

// all menu lines are grouped by a vlist element
M2_LIST(list_controller_menu) = { &list_controller_light, &list_controller_power};
M2_VLIST(el_controller_menu_vlist, NULL, list_controller_menu);

// center the menu on the display
M2_ALIGN(el_top_controller_menu, "-1|1W64H64", &el_controller_menu_vlist);


/*======================================================================*/
//=======================================
// utility procedure to convert values
void pwm_value_to_str(uint8_t value, char *dest) {
  switch(value) {
    case 0: strcpy(dest, "low"); break;
    case 1: strcpy(dest, "25%"); break;
    case 2: strcpy(dest, "50%"); break;
    case 3: strcpy(dest, "75%"); break;
    case 4: strcpy(dest, "high"); break;
  }
}

uint8_t pwm_value_to_analog(uint8_t value) {
  switch(value) {
    case 0: return 0;
    case 1: return 63;
    case 2: return 127;
    case 3: return 191;
  }
  return 255;
}

//=======================================
// array for the pwm information of all pwm pins 

// array with all the information
#define PWM_PIN_CNT 6
uint8_t pwm_duty_array[PWM_PIN_CNT] = { 0,0,0,0,0,0 };
uint8_t pwm_pin_array[PWM_PIN_CNT] = { 3,5,6,9,10,11 };

//=======================================
// variables for the user interface

// the current index contains the position within the array which gets presented to the user
uint8_t pwm_menu_current_index = 0;

// values for the menu; will be modified by the user
// transfer values from the array to these variables: pwm_prepare_user_input
// transfer values from these variables to the array: pwm_apply_user_input
uint8_t pwm_menu_duty = 0;
uint8_t pwm_menu_pin = 0;

//=======================================
// transfer procedure between menu variables and the array with pwm information

// get pin and value pair from the global array and store them in the menu variables
void pwm_prepare_user_input(void) {
  pwm_menu_duty = pwm_duty_array[pwm_menu_current_index];
  pwm_menu_pin = pwm_pin_array[pwm_menu_current_index];
}

// write user input back to the array and to the analog pin
void pwm_apply_user_input(void) {
  // write user input into array
  pwm_duty_array[pwm_menu_current_index] = pwm_menu_duty;
  pwm_pin_array[pwm_menu_current_index] = pwm_menu_pin;
  // apply user input to the hardware
  // analogWrite(pwm_menu_pin, pwm_value_to_analog(pwm_menu_value));
}

//=======================================
// forward declaration of the pin selection menu
M2_EXTERN_ALIGN(top_el_pin_list);

//=======================================
// the menu / user interface for one pwm pin

// this procedure is called by the "ok" button
void pwm_fn_ok(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  // finish user entry
  pwm_apply_user_input();
  
  // go back to parent menu
  m2_SetRootExtended(&top_el_pin_list, pwm_menu_current_index, 0);
}

// this will return a user readable string for the internal value
const char *pwm_fn_duty(uint8_t idx) {
  static char buf[8];
  pwm_value_to_str(idx, buf);
  return buf;
}

// the arduino pin number is read only for the user (info field)
M2_LABEL(el_pwm_pin_label, NULL, "Pin:");
M2_U8NUM(el_pwm_pin_num, "r1c2", 0, 255, &pwm_menu_pin);

// the duty field can be changed by the user
M2_LABEL(el_pwm_duty_label, NULL, "Duty: ");
M2_COMBO(el_pwm_duty, NULL, &pwm_menu_duty, 5, pwm_fn_duty);

// cancel: do not store user values, go back directly
M2_ROOT(el_pwm_cancel, "f4n3", "Cancel", &top_el_pin_list);

// ok: write user values back to the array and apply values to the hardware
M2_BUTTON(el_pwm_ok, "f4", "Ok", pwm_fn_ok);

// the following grid-list will arrange the elements on the display
M2_LIST(list_pwm_menu) = { 
    &el_pwm_pin_label, &el_pwm_pin_num, 
    &el_pwm_duty_label, &el_pwm_duty,  
    &el_pwm_cancel, &el_pwm_ok 
};
M2_GRIDLIST(el_pwm_grid, "c2", list_pwm_menu);

// center the menu on the display
M2_ALIGN(el_top_pwm_menu, "-1|1W64H64", &el_pwm_grid);


//=======================================
// this menu selects one of the pwm pins (implemented with the STRLIST element)
uint8_t pin_list_first = 0;
uint8_t pin_list_cnt = PWM_PIN_CNT;

// callback procedure for the STRLIST element
const char *pin_list_cb(uint8_t idx, uint8_t msg) {
  static char s[12];
  s[0] = '\0';
  if ( msg == M2_STRLIST_MSG_SELECT ) {
    // the user has selected a pin, prepare sub menu
    
    // inform the pwm dialog which array index has to be modfied
    pwm_menu_current_index = idx;
    
    // transfer values from the array to the menu variables
    pwm_prepare_user_input();
    
    // give control to the pwm dialog
    m2_SetRoot(&el_top_pwm_menu);
  } else {
    // convert the idx into some readable line for the user
    strcpy(s, "Pin ");
    strcpy(s+4, "x");
    // add a space
    s[6] = ' ';
    // also show the current duty
    pwm_value_to_str(pwm_duty_array[idx], s+7);
  }
  return s;
}

// selection menu for the pins, composed of a STRLIST element with a scroll bar
M2_STRLIST(el_pin_list_strlist, "l3W56", &pin_list_first, &pin_list_cnt, pin_list_cb);
M2_SPACE(el_pin_list_space, "W1h1");
M2_VSB(el_pin_list_vsb, "l3W4r1", &pin_list_first, &pin_list_cnt);
M2_LIST(list_pin_list) = { &el_pin_list_strlist, &el_pin_list_space, &el_pin_list_vsb };
M2_HLIST(el_pin_list_hlist, NULL, list_pin_list);
M2_ALIGN(top_el_pin_list, "-1|1W64H64", &el_pin_list_hlist);



/*======================================================================*/
/* bookmarks */

char bm_m1_name[] = "Menu 1";
M2_EXTERN_ALIGN(el_bm_m1);
char bm_m2_name[] = "Menu 2";
M2_EXTERN_ALIGN(el_bm_m2);
char bm_m3_name[] = "Menu 3";
M2_EXTERN_ALIGN(el_bm_m3);
char bm_m4_name[] = "Menu 4";
M2_EXTERN_ALIGN(el_bm_m4);

char bm_m5_name[] = "Menu 5";
M2_EXTERN_ALIGN(el_bm_m5);
char bm_m6_name[] = "Menu 6";
M2_EXTERN_ALIGN(el_bm_m6);
char bm_m7_name[] = "Menu 7";
M2_EXTERN_ALIGN(el_bm_m7);
char bm_m8_name[] = "Menu 8";
M2_EXTERN_ALIGN(el_bm_m8);


#define BM_MAX 6
const char *bm_name[BM_MAX] = { "", "", "", "", "", "" };
m2_rom_void_p  bm_menu[BM_MAX] = { NULL, NULL, NULL, NULL, NULL, NULL };
const char *bm_last_name = "";
m2_rom_void_p bm_last_menu = NULL;

/* variables for the STRLIST element with all bookmarks */
uint8_t bm_list_cnt = BM_MAX;
uint8_t bm_list_first = 0;

/* Is this menu already a bookmark? */
uint8_t bm_is_bookmark(m2_rom_void_p menu) {
  uint8_t i;
  for ( i = 0; i < BM_MAX; i++ ) {
    if ( bm_menu[i] ==  menu )
      return 1;
  }
  return 0;
}

/* Add a new bookmark the bookmark list. Add the new bookmark at the beginning of the list. */
void bm_add_menu(const char *name, m2_rom_void_p menu) {
  uint8_t i;
  for ( i = BM_MAX-1; i > 0; i-- ) {
    bm_name[i] = bm_name[i-1];
    bm_menu[i] = bm_menu[i-1];
  }
  bm_name[0] = name;
  bm_menu[0] = menu;
}

/* root change callback to support the bookmark menu */
void bm_root_change_cb(m2_rom_void_p new_root, m2_rom_void_p old_root, uint8_t change_value)
{
	(void)change_value; 
	(void)new_root; 
  bm_last_name = "";
  if ( old_root == &el_bm_m1 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m1_name;
  }
  if ( old_root == &el_bm_m2 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m2_name;
  }
  if ( old_root == &el_bm_m3 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m3_name;
  }
  if ( old_root == &el_bm_m4 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m4_name;
  }
  if ( old_root == &el_bm_m5 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m5_name;
  }
  if ( old_root == &el_bm_m6 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m6_name;
  }
  if ( old_root == &el_bm_m7 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m7_name;
  }
  if ( old_root == &el_bm_m8 )  {
    bm_last_menu = old_root;
    bm_last_name = bm_m8_name;
  }
  printf("bookmark root change (last = '%s')\n", bm_last_name);
  
}

/*--- Home Menu ---*/

void bm_add_bookmark_button_function(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
    if ( bm_last_menu != NULL ) {
      /* check if the menu is already bookmarked */
      if ( bm_is_bookmark(bm_last_menu) == 0 ) {
	/* if not, add it to the list */
	bm_add_menu(bm_last_name, bm_last_menu);
      }
      m2_SetRoot(bm_last_menu);
    }
}

void bm_return_to_last_menu_cb(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
    if ( bm_last_menu != NULL )
      m2_SetRoot(bm_last_menu);
}

const char *bm_list_cb(uint8_t idx, uint8_t msg) {
  if ( msg == M2_STRLIST_MSG_SELECT ) {
    if ( bm_menu[idx] != NULL ) {
      m2_SetRoot(bm_menu[idx]);
    }
  }
  return bm_name[idx];
}

M2_LABEL(el_bm_home_label, NULL, "Last Menu:");
M2_BUTTONPTR(el_bm_home_last, "f4", &bm_last_name, bm_return_to_last_menu_cb);
M2_BUTTON(el_bm_add, "f4", "Add as bookmark", bm_add_bookmark_button_function);


M2_STRLIST(el_bm_list_strlist, "l3W56", &bm_list_first, &bm_list_cnt, bm_list_cb);
M2_SPACE(el_bm_list_space, "W1h1");
M2_VSB(el_bm_list_vsb, "l3W4r1", &bm_list_first, &bm_list_cnt);
M2_LIST(list_bm_list) = { &el_bm_list_strlist, &el_bm_list_space, &el_bm_list_vsb };
M2_HLIST(el_bm_list_hlist, NULL, list_bm_list);

M2_LIST(list_bm_home) = { &el_bm_home_label, &el_bm_home_last, &el_bm_add, &el_bm_list_hlist };
M2_VLIST(el_bm_home_vl, NULL, list_bm_home);
M2_ALIGN(el_bm_home, "W64H64", &el_bm_home_vl);


/*--- Menu 1 ---*/
M2_LABEL(el_bm_m1_label, NULL, bm_m1_name);
M2_LABEL(el_bm_m1_home_info, NULL, "Press Home Button");
M2_ROOT(el_bm_m1_b1, "f4", bm_m2_name, &el_bm_m2);
M2_ROOT(el_bm_m1_b2, "f4", bm_m5_name, &el_bm_m5);
M2_LIST(list_bm_m1) = { &el_bm_m1_label, &el_bm_m1_home_info, &el_bm_m1_b1, &el_bm_m1_b2 };
M2_VLIST(el_bm_m1_vl, NULL, list_bm_m1);
M2_ALIGN(el_bm_m1, "W64H64", &el_bm_m1_vl);

/*--- Menu 2 ---*/
M2_LABEL(el_bm_m2_label, NULL, bm_m2_name);
M2_ROOT(el_bm_m2_b1, "f4", bm_m7_name, &el_bm_m7);
M2_ROOT(el_bm_m2_b2, "f4", bm_m3_name, &el_bm_m3);
M2_LIST(list_bm_m2) = { &el_bm_m2_label, &el_bm_m2_b1, &el_bm_m2_b2 };
M2_VLIST(el_bm_m2_vl, NULL, list_bm_m2);
M2_ALIGN(el_bm_m2, "W64H64", &el_bm_m2_vl);

/*--- Menu 3 ---*/
M2_LABEL(el_bm_m3_label, NULL, bm_m3_name);
M2_ROOT(el_bm_m3_b1, "f4", bm_m4_name, &el_bm_m4);
M2_ROOT(el_bm_m3_b2, "f4", bm_m8_name, &el_bm_m8);
M2_LIST(list_bm_m3) = { &el_bm_m3_label, &el_bm_m3_b1, &el_bm_m3_b2 };
M2_VLIST(el_bm_m3_vl, NULL, list_bm_m3);
M2_ALIGN(el_bm_m3, "W64H64", &el_bm_m3_vl);

/*--- Menu 4 ---*/
M2_LABEL(el_bm_m4_label, NULL, bm_m4_name);
M2_ROOT(el_bm_m4_b1, "f4", bm_m5_name, &el_bm_m5);
M2_ROOT(el_bm_m4_b2, "f4", bm_m3_name, &el_bm_m3);
M2_LIST(list_bm_m4) = { &el_bm_m4_label, &el_bm_m4_b1, &el_bm_m4_b2 };
M2_VLIST(el_bm_m4_vl, NULL, list_bm_m4);
M2_ALIGN(el_bm_m4, "W64H64", &el_bm_m4_vl);

/*--- Menu 5 ---*/
M2_LABEL(el_bm_m5_label, NULL, bm_m5_name);
M2_ROOT(el_bm_m5_b1, "f4", bm_m2_name, &el_bm_m2);
M2_ROOT(el_bm_m5_b2, "f4", bm_m6_name, &el_bm_m6);
M2_LIST(list_bm_m5) = { &el_bm_m5_label, &el_bm_m5_b1, &el_bm_m5_b2 };
M2_VLIST(el_bm_m5_vl, NULL, list_bm_m5);
M2_ALIGN(el_bm_m5, "W64H64", &el_bm_m5_vl);

/*--- Menu 6 ---*/
M2_LABEL(el_bm_m6_label, NULL, bm_m6_name);
M2_ROOT(el_bm_m6_b1, "f4", bm_m5_name, &el_bm_m5);
M2_ROOT(el_bm_m6_b2, "f4", bm_m7_name, &el_bm_m7);
M2_LIST(list_bm_m6) = { &el_bm_m6_label, &el_bm_m6_b1, &el_bm_m6_b2 };
M2_VLIST(el_bm_m6_vl, NULL, list_bm_m6);
M2_ALIGN(el_bm_m6, "W64H64", &el_bm_m6_vl);

/*--- Menu 7 ---*/
M2_LABEL(el_bm_m7_label, NULL, bm_m7_name);
M2_ROOT(el_bm_m7_b1, "f4", bm_m1_name, &el_bm_m1);
M2_ROOT(el_bm_m7_b2, "f4", bm_m3_name, &el_bm_m3);
M2_LIST(list_bm_m7) = { &el_bm_m7_label, &el_bm_m7_b1, &el_bm_m7_b2 };
M2_VLIST(el_bm_m7_vl, NULL, list_bm_m7);
M2_ALIGN(el_bm_m7, "W64H64", &el_bm_m7_vl);

/*--- Menu 8 ---*/
M2_LABEL(el_bm_m8_label, NULL, bm_m8_name);
M2_ROOT(el_bm_m8_b1, "f4", bm_m6_name, &el_bm_m6);
M2_ROOT(el_bm_m8_b2, "f4", bm_m2_name, &el_bm_m2);
M2_LIST(list_bm_m8) = { &el_bm_m8_label, &el_bm_m8_b1, &el_bm_m8_b2 };
M2_VLIST(el_bm_m8_vl, NULL, list_bm_m8);
M2_ALIGN(el_bm_m8, "W64H64", &el_bm_m8_vl);
/*======================================================================*/

/*======================================================================*/
/*=== SPACECB with 2LMENU ===*/

M2_EXTERN_ALIGN(top_el_spacecb_menu);

uint8_t el_space_u8 = 0;

void fn_space_cb_zero(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  el_space_u8 = 0;
  m2_SetRootExtended(&top_el_spacecb_menu, 0, 0);
}

void fn_space_cb_inc(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  puts("inc");
  el_space_u8++;
  m2_SetRootExtended(&top_el_spacecb_menu, 1, 0);
}

void fn_space_cb_dec(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  el_space_u8--;
  m2_SetRootExtended(&top_el_spacecb_menu, 2, 0);
}

M2_SPACECB(el_space_cb_zero, NULL, fn_space_cb_zero);
M2_SPACECB(el_space_cb_inc, NULL, fn_space_cb_inc);
M2_SPACECB(el_space_cb_dec, NULL, fn_space_cb_dec);

m2_menu_entry space_cb_menu_data[] = 
{
  { "Zero", &el_space_cb_zero },
  { "Inc", &el_space_cb_inc },
  { "Dec", &el_space_cb_dec },
  { NULL, NULL },
};

uint8_t el_2lspace_first = 0;
uint8_t el_2lspace_cnt = 3;

/* for m2icon fonts, 65: closed folder, 102: open folder */
M2_U8NUM(el_2lspace_u8, "r1c3", 0, 255, &el_space_u8);
M2_2LMENU(el_2lspace_strlist, "l3e15F3W47", &el_2lspace_first, &el_2lspace_cnt, space_cb_menu_data, 65, 102, '\0');
//M2_SPACE(el_2lspace_space, "W1h1");
//M2_VSB(el_2lspace_vsb, "l4W4r1", &el_2lspace_first, &el_2lspace_cnt);
M2_LIST(list_2lspace_strlist) = { &el_2lspace_u8, &el_2lspace_strlist };
M2_VLIST(el_2lspace_hlist, NULL, list_2lspace_strlist);
M2_ALIGN(top_el_spacecb_menu, "-1|1W64H64", &el_2lspace_hlist);

/*======================================================================*/
/* combo ptr */

uint8_t comboptr_val = 0;
uint8_t comboptr_cnt = 3;
const char *comboptr_idx_to_color(uint8_t idx)
{
  if ( idx == 0 )
    return "red";
  else if (idx == 1 )
    return "green";
  return "blue";
}
M2_COMBOPTR(el_comboptr, NULL, &comboptr_val, &comboptr_cnt, comboptr_idx_to_color);
M2_ALIGN(top_el_comboptr_menu, "W64H64", &el_comboptr);

/*======================================================================*/
/* similar submenus with different inputs */


/*--- variables to store user input ---*/
uint8_t sisu_w;
uint8_t sisu_x;
uint8_t sisu_y;
uint8_t sisu_z;

/*--- forward declaration of the top menu */
M2_EXTERN_ALIGN(top_el_sisu);

/*--- labels end entry fields for user input ---*/
/*--- w ---*/
M2_LABEL(el_sisu_label_w, NULL, "w:");
M2_U8NUM(el_sisu_field_w, "c2", 0,99,&sisu_w);
/*--- x ---*/
M2_LABEL(el_sisu_label_x, NULL, "x:");
M2_U8NUM(el_sisu_field_x, "c2", 0,99,&sisu_x);
/*--- y ---*/
M2_LABEL(el_sisu_label_y, NULL, "y:");
M2_U8NUM(el_sisu_field_y, "c2", 0,99,&sisu_y);
/*--- z ---*/
M2_LABEL(el_sisu_label_z, NULL, "z:");
M2_U8NUM(el_sisu_field_z, "c2", 0,99,&sisu_z);

/*--- global buttons ---*/
M2_ROOT(el_sisu_cancel, "f4", "Cancel", &top_el_sisu);

/*--- submenu: w, y, z ---*/
void fn_sisu_b1(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  sisu_x = 0;	// default value for "w"
  // process values here
  m2_SetRoot(&top_el_sisu);
}
M2_BUTTON(el_sisu_b1, "f4", " ok ", fn_sisu_b1);
M2_LIST(list_sisu_m1) = { 
  &el_sisu_label_w, &el_sisu_field_w, 
  &el_sisu_label_y, &el_sisu_field_y, 
  &el_sisu_label_z, &el_sisu_field_z, 
  &el_sisu_cancel, &el_sisu_b1
  };
M2_GRIDLIST(el_sisu_grid_m1, "c2",list_sisu_m1);
M2_ALIGN(top_el_sisu_m1, "W64H64", &el_sisu_grid_m1);

/*--- submenu: x, y, z ---*/
void fn_sisu_b2(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  sisu_w = 0;	// default value for "w"
  // process values here
  m2_SetRoot(&top_el_sisu);
}
M2_BUTTON(el_sisu_b2, "f4", " ok ", fn_sisu_b2);
M2_LIST(list_sisu_m2) = { 
  &el_sisu_label_x, &el_sisu_field_x, 
  &el_sisu_label_y, &el_sisu_field_y, 
  &el_sisu_label_z, &el_sisu_field_z, 
  &el_sisu_cancel, &el_sisu_b2
  };
M2_GRIDLIST(el_sisu_grid_m2, "c2",list_sisu_m2);
M2_ALIGN(top_el_sisu_m2, "W64H64", &el_sisu_grid_m2);
  
/*--- submenu: w, x, y, z ---*/
void fn_sisu_b3(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  sisu_z = 0;	// default value for "z"
  // process values here
  m2_SetRoot(&top_el_sisu);
}
M2_BUTTON(el_sisu_b3, "f4", " ok ", fn_sisu_b3);
M2_LIST(list_sisu_m3) = { 
  &el_sisu_label_w, &el_sisu_field_w, 
  &el_sisu_label_x, &el_sisu_field_x, 
  &el_sisu_label_y, &el_sisu_field_y, 
  &el_sisu_cancel, &el_sisu_b3
  };
M2_GRIDLIST(el_sisu_grid_m3, "c2",list_sisu_m3);
M2_ALIGN(top_el_sisu_m3, "W64H64", &el_sisu_grid_m3);
  
/*--- main menu ---*/
M2_ROOT(el_sisu_c1, "f4", "Menu 1", &top_el_sisu_m1);
M2_ROOT(el_sisu_c2, "f4", "Menu 2", &top_el_sisu_m2);
M2_ROOT(el_sisu_c3, "f4", "Menu 3", &top_el_sisu_m3);
M2_LIST(list_sisu) = { &el_sisu_c1, &el_sisu_c2, &el_sisu_c3};
M2_VLIST(el_sisu_vl, NULL, list_sisu);
M2_ALIGN(top_el_sisu, NULL, &el_sisu_vl);

/*======================================================================*/

M2_ROOT(el_align_check, "f4", "Back", &top_el_tlsm);
M2_ALIGN(top_el_align_check, "-0|0w128h64", &el_align_check);

//M2_ROOT(top_el_align_check, "f4", "Back", &top_el_tlsm);

uint8_t el_align_check_first = 0;
uint8_t el_align_check_cnt = 10;
//M2_VSB(top_el_align_check, "l3w6r1", &el_align_check_first, &el_align_check_cnt);






/*======================================================================*/
/*=== Extended 2L Menu ===*/

M2_EXTERN_ALIGN(top_el_x2l_menu);

const char *xmenu_cb(uint8_t idx, uint8_t msg)
{
	printf("xmenu cb idx:%d msg:%d\n", idx, msg);
	if ( msg == M2_STRLIST_MSG_GET_STR && idx == 0 )
	{
		return "cb top label 0";
	}
	if ( msg == M2_STRLIST_MSG_SELECT && idx == 1 )
	{
		m2_SetRoot(&top_el_x2l_menu);
	}
	if ( msg == M2_STRLIST_MSG_GET_STR && idx == 1 )
	{
		return " cb label 1";
	}
	if ( msg == M2_STRLIST_MSG_SELECT && idx == 2 )
	{
		m2_SetRoot(&top_el_x2l_menu);
	}
	if ( msg == M2_STRLIST_MSG_GET_STR && idx == 3 )
	{
		return "=Menu 2=";
	}
	return "";
}

m2_xmenu_entry xmenu_data[] = 
{
  { "", NULL, xmenu_cb },	/* menu label for this line is returned by xmenu_cb */
  { ".", &top_el_tlsm, xmenu_cb },		/* menu label for this line is returned by xmenu_cb */
  { ". Sub 1-2", &top_el_tlsm, xmenu_cb },
  { "", &top_el_tlsm, xmenu_cb },	/* menu label for this line is returned by xmenu_cb */
  { "Menu 3", NULL, xmenu_cb },
  { ". Sub 3-1", &top_el_tlsm, xmenu_cb },
  { ". Sub 3-2", &top_el_tlsm, xmenu_cb },
  { "Menu 4", &top_el_tlsm, xmenu_cb },
  { "Menu 5", NULL, xmenu_cb },
  { ". Sub 5-1", &top_el_tlsm, xmenu_cb },
  { ". Sub 5-2", &top_el_tlsm, xmenu_cb },
  { ". Sub 5-3", &top_el_tlsm, xmenu_cb },
  { NULL, NULL, NULL },
};

uint8_t el_x2l_first = 0;
uint8_t el_x2l_cnt = 3;

/* for m2icon fonts, 65: closed folder, 102: open folder */
M2_X2LMENU(el_x2l_strlist, "l4e15F3W47", &el_x2l_first, &el_x2l_cnt, xmenu_data, 65, 102, '\0');
M2_SPACE(el_x2l_space, "W1h1");
M2_VSB(el_x2l_vsb, "l4W4r1", &el_x2l_first, &el_x2l_cnt);
M2_LIST(list_x2l) = { &el_x2l_strlist, &el_x2l_space, &el_x2l_vsb };
M2_HLIST(el_x2l_hlist, NULL, list_x2l);
M2_ALIGN(top_el_x2l_menu, "-1|1W64H64", &el_x2l_hlist);




/*======================================================================*/
/* hide example */

uint8_t hide_on = 0;
uint8_t hide_u8num = 0;


M2_LABEL(el_hide_label, NULL, "Hide: ");
M2_TOGGLE(el_hide_toggle, "", &hide_on);

M2_LABEL(el_hide_num_label, NULL, "Num: ");
M2_HIDE(h_el_hide_num_label, NULL, &hide_on, &el_hide_num_label);
M2_U8NUM(el_hide_num_u8, "c2", 0, 99, &hide_u8num);
M2_HIDE(h_el_hide_num_u8, NULL, &hide_on, &el_hide_num_u8);

M2_LIST(hide_example_list) = { 
    &el_hide_label, &el_hide_toggle, 
    &h_el_hide_num_label, &h_el_hide_num_u8
};
M2_GRIDLIST(el_hide_example, "c2", hide_example_list);
M2_ALIGN(el_top_hide_example, "-1|1W64H64", &el_hide_example);


/*======================================================================*/
/* issue 111 */
/*
#define i111_u8g_logo_width 38
#define i111_u8g_logo_height 24
//static unsigned char u8g_logo_bits[] = {
static char i111_u8g_logo_bits[] U8G_PROGMEM = {
0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xe0, 0xe0,
0xff, 0xff, 0x3f, 0xe3, 0xe1, 0xff, 0xff, 0x3f, 0xf3, 0xf1, 0xff, 0xff,
0x3f, 0xf3, 0xf1, 0xfe, 0xbf, 0x37, 0xf3, 0x11, 0x1c, 0x1f, 0x30, 0xf3,
0x01, 0x08, 0x8c, 0x20, 0xf3, 0x01, 0x00, 0xc0, 0x39, 0xf3, 0x81, 0xc7,
0xc1, 0x39, 0xf3, 0xc1, 0xc7, 0xc9, 0x38, 0xf3, 0xc1, 0xc3, 0x19, 0x3c,
0xe3, 0x89, 0x01, 0x98, 0x3f, 0xc7, 0x18, 0x00, 0x08, 0x3e, 0x0f, 0x3c,
0x70, 0x1c, 0x30, 0x3f, 0xff, 0xfc, 0x87, 0x31, 0xff, 0xff, 0xbf, 0xc7,
0x23, 0x01, 0x00, 0x00, 0xc6, 0x23, 0x03, 0x00, 0x00, 0x0e, 0x30, 0xff,
0xff, 0x3f, 0x1f, 0x3c, 0xff, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0xff, 0x3f,
0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x3f };

M2_XBMLABELP(i111_hello_world_label, "x64y32", i111_u8g_logo_width, i111_u8g_logo_height, i111_u8g_logo_bits);
M2_LIST(i111_list) = {&i111_hello_world_label};
M2_XYLIST(i111_list_element, NULL,i111_list);
*/

/*======================================================================*/
/* quick key menues */

M2_LABEL(el_qknum_label, NULL, "q1:");
M2_U8NUM(el_qknum_val, "q1", 0, 255, &u8num);
M2_LIST(list_qknum) = { &el_qknum_label, &el_qknum_val };
M2_HLIST(el_qknum_hlist, "", list_qknum);
M2_ROOT(el_qknum_goto_top, "f4q6", "q6: main menu", &top_el_tlsm);
M2_LIST(list_qknum_v) = { &el_qknum_hlist, &el_qknum_goto_top };
M2_VLIST(el_qknum_vlist, NULL, list_qknum_v);
M2_ALIGN(top_el_qknum, "-1|1W64H64", &el_qknum_vlist);

uint8_t qktoggle_val = 0;
M2_LABEL(el_qktoggle_label, NULL, "q1:");
M2_TOGGLE(el_qktoggle_val, "q1", &qktoggle_val);
M2_LIST(list_qktoggle) = { &el_qktoggle_label, &el_qktoggle_val };
M2_HLIST(el_qktoggle_hlist, "", list_qktoggle);
M2_ROOT(el_qktoggle_goto_top, "f4q2", "q2: main menu", &top_el_tlsm);
M2_LIST(list_qktoggle_v) = { &el_qktoggle_hlist, &el_qktoggle_goto_top };
M2_VLIST(el_qktoggle_vlist, NULL, list_qktoggle_v);
M2_ALIGN(top_el_qktoggle, "-1|1W64H64", &el_qktoggle_vlist);


M2_ROOT(el_qk_menu1, "f4q1", "q1:U8NUM", &top_el_qknum);
M2_ROOT(el_qk_menu2, "f4q2", "q2:TOGGLE", &top_el_qktoggle);
M2_LIST(list_qk_menu) = { &el_qk_menu1, &el_qk_menu2 };
M2_VLIST(el_qk_menu_vlist, NULL, list_qk_menu);
M2_ALIGN(top_el_quick_key, "-1|1W64H64", &el_qk_menu_vlist);


/*======================================================================*/
/* combofn */

uint8_t combofn_color = 0;
uint8_t combofn_priority = 0;

const char *combofn_color_cb(m2_rom_void_p element, uint8_t msg, uint8_t *valptr)
{
	(void)element; 
	switch(msg)
	{
		case M2_COMBOFN_MSG_GET_VALUE:
			*valptr = combofn_color;
			break;
		case M2_COMBOFN_MSG_SET_VALUE:
			combofn_color = *valptr;
			break;
		case M2_COMBOFN_MSG_GET_STRING:
			if ( *valptr == 0 )
				return "orange";
			else if (*valptr == 1 )
				return "yellow";
			return "cyan";
	}
	return NULL;
}

const char *combofn_priority_cb(m2_rom_void_p element, uint8_t msg, uint8_t *valptr)
{
	(void)element; 
	switch(msg)
	{
		case M2_COMBOFN_MSG_GET_VALUE:
			*valptr = combofn_priority;
			break;
		case M2_COMBOFN_MSG_SET_VALUE:
			combofn_priority = *valptr;
			break;
		case M2_COMBOFN_MSG_GET_STRING:
			switch(*valptr)
			{
				case 0: return "lowest";
				case 1: return "low";
				case 2: return "medium";
				case 3: return "high";
				case 4: return "highest";
			}
			return "";
	}
	return NULL;
}


M2_LABEL(el_labelcombofn1, NULL, "Color:");
M2_COMBOFN(el_combofn1, NULL, 3, combofn_color_cb);

M2_LABEL(el_labelcombofn2, NULL, "Priority: ");
M2_COMBOFN(el_combofn2, "v1", 5, combofn_priority_cb);

M2_LIST(list_combofn) = { 
    &el_labelcombofn1, &el_combofn1, 
    &el_labelcombofn2, &el_combofn2,  
    &el_cancel, &el_ok 
};
M2_GRIDLIST(el_top_combofn, "c2", list_combofn);


/*======================================================================*/

const char *svo_pos_str = "<keine>";
int svo_pos = 0;


/* svo_hm: Servo Untermenue */

M2_EXTERN_VLIST(top_el_svo_um);

void fn_svo_pos1(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  svo_pos_str = "Servostellung 1";
  svo_pos = 1;
  m2_SetRoot(&top_el_svo_um);
}
void fn_svo_pos2(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  svo_pos_str = "Servostellung 2";
  svo_pos = 2;
  m2_SetRoot(&top_el_svo_um);
}
void fn_svo_pos3(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  svo_pos_str = "Servostellung 3";
  svo_pos = 3;
  m2_SetRoot(&top_el_svo_um);
}
void fn_svo_pos4(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  svo_pos_str = "Servostellung 4";
  svo_pos = 4;
  m2_SetRoot(&top_el_svo_um);
}

M2_BUTTON(el_svo_um_pos1, NULL, "Servostellung 1", &fn_svo_pos1);
M2_BUTTON(el_svo_um_pos2, NULL, "Servostellung 2", &fn_svo_pos2);
M2_BUTTON(el_svo_um_pos3, NULL, "Servostellung 3", &fn_svo_pos3);
M2_BUTTON(el_svo_um_pos4, NULL, "Servostellung 4", &fn_svo_pos4);
M2_LIST(list_svo_um) = { &el_svo_um_pos1, &el_svo_um_pos2, &el_svo_um_pos3, &el_svo_um_pos4};
M2_VLIST(top_el_svo_um, NULL, list_svo_um);


/* svo_hm: Servo Hauptmenue */

M2_LABEL(el_svo_hm_title, "f8W64", "Servoeinstellung");
M2_ROOT(el_svo_hm_jmp_um, "f8W64", "Auswahl", &top_el_svo_um);

M2_LABEL(el_svo_hm_space, NULL, "");
M2_LABELPTR(el_svo_hm_pos, "f8W64", &svo_pos_str);

M2_LIST(list_svo_hm) = { &el_svo_hm_title, &el_svo_hm_jmp_um, &el_svo_hm_space, &el_svo_hm_pos};
M2_VLIST(el_svo_hm_vlist, NULL, list_svo_hm);
M2_ALIGN(top_el_svo_hm, "-1|1W64H64", &el_svo_hm_vlist);


/*======================================================================*/
/* top level sdl menu: top_el_tlsm */


const char *el_tlsm_strlist_cb(uint8_t idx, uint8_t msg) {
  const char *s = "";

  if ( idx == 0 ) {
    s = "Select Style";
    //if ( msg == M2_STRLIST_MSG_SELECT )
    //  m2_SetRoot(&top_el_gfx);
  }
  else if ( idx == 1 ) {
    s = "Adjust Style";
    //if ( msg == M2_STRLIST_MSG_SELECT )
    // m2_SetRoot(&top_el_xwp);
  }  
  else if ( idx == 2 ) {
    s = "Select Font";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&top_el_fsel);*/
  }  
  else if ( idx == 3 ) {
    s = "Buttons";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&el_top_btn);
  }
  else if ( idx == 4 ) {
    s = "Text Entry";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&el_top_te);
  }
  else if ( idx == 5 ) {
    s = "Radio";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&el_top_rb);
  }
  else if ( idx == 6 ) {
    s = "Combo";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&el_top_combo);
  }
  else if ( idx == 7 ) {
    s = "Number Entry";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&el_num_menu);
  }
  else if ( idx == 8 ) {
    s = "Hello World";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&hello_world_label);
  }
  else if ( idx == 9 ) {
    s = "U32Plain";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&el_u32num);
  }
  else if ( idx == 10 ) {
    s = "StrList";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&top_el_strlist);
  }
  else if ( idx == 11 ) {
    s = "MultiSelect";
    if ( msg == M2_STRLIST_MSG_SELECT )
      m2_SetRoot(&top_el_muse);
  }
	else if ( idx == 12 ) {
    s = "FileSelection";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_top_fs);
    }*/
  }
  else if ( idx == 13 ) {
    s = "Save As";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_sa);
    }
  }
  else if ( idx == 14 ) {
    s = "Speed";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_speed);
    }
  }
  else if ( idx == 15 ) {
    s = "Menues";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_select_menu);
    }
  }
  else if ( idx == 16 ) {
    s = "FileSelection 2";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_top_fs2);
    }*/
  }
  else if ( idx == 17 ) {
    s = "Date";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_top_dt);
    }
  }
  else if ( idx == 18 ) {
    s = "Forum Jun 2012";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_top_controller_menu);
    }
  }
  else if ( idx == 19 ) {
    s = "PWM Menu";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_pin_list);
    }
  }
  else if ( idx == 20 ) {
    s = "Bookmarks";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetFont(0, (const void *)u8g_font_6x10);
      m2_SetHome(&el_bm_home);
      m2_SetHome2(&el_bm_home);
      m2_SetRootChangeCallback(bm_root_change_cb);
      m2_SetRoot(&el_bm_m1);
    }*/
  }
  else if ( idx == 21 ) {
    s = "XBM";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_xbm);
    }*/
  }
  else if ( idx == 22 ) {
    s = "SPACE CB";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_spacecb_menu);
    }    
  }
  else if ( idx == 23 ) {
    s = "COMBOPTR";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_comboptr_menu);
    }    
  }
  else if ( idx == 24 ) {
    s = "SiSu";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_sisu);
    }    
  }
  else if ( idx == 25 ) {
    s = "TSK U8 U32";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_tsnum_menu);
    }    
  }
  else if ( idx == 26 ) {
    s = "TS Menu";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_ts_mnu);
    }    
  }
  else if ( idx == 27 ) {
    s = "TSK U32 Input";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_tsk_num_menu);
    }    
  }
  else if ( idx == 28 ) {
    s = "TSK U8 Input";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_ts_u8_menu);
    }    
  }
  else if ( idx == 29 ) {
    s = "Align (Issue 95)";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_align_check);
    }    
  }
  else if ( idx == 30 ) {
    s = "X2LMENU";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_x2l_menu);
    }    
  }
  else if ( idx == 31 ) {
    s = "XBMTSK";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_xtsk_num_menu);
    } */   
  }
  else if ( idx == 32 ) {
    s = "XBMTSK BOX";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_xtsk_num_box_menu);
    }  */  
  }
  else if ( idx == 33 ) {
    s = "HIDE";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_top_hide_example );
    }    
  }
  else if ( idx == 34 ) {
    s = "Issue 111";
    /*if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&i111_list_element );
    } */   
  }
  else if ( idx == 35 ) {
    s = "Quick Key";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_quick_key);
    }    
  }
  else if ( idx == 36 ) {
    s = "COMBOFN";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&el_top_combofn);
    }    
  }
  else if ( idx == 37 ) {
    s = "ServoPos";
    if ( msg == M2_STRLIST_MSG_SELECT )
    {
      m2_SetRoot(&top_el_svo_hm);
    }    
  }

  
 
  
  
  
  
  
  return s;
}


uint8_t el_tlsm_first = 0;
uint8_t el_tlsm_cnt = 38;

M2_STRLIST(el_tlsm_strlist, "l8W56t1", &el_tlsm_first, &el_tlsm_cnt, el_tlsm_strlist_cb);
M2_SPACE(el_tlsm_space, "W1h1");
M2_VSB(el_tlsm_vsb, "l8W4r1t1", &el_tlsm_first, &el_tlsm_cnt);
M2_LIST(list_tlsm_strlist) = { &el_tlsm_strlist, &el_tlsm_space, &el_tlsm_vsb };
M2_HLIST(el_tlsm_hlist, NULL, list_tlsm_strlist);
M2_ALIGN(top_el_tlsm, "-1|1W64H64", &el_tlsm_hlist);

/*======================================================================*/

void generic_root_change_cb(m2_rom_void_p new_root, m2_rom_void_p old_root, uint8_t change_value)
{
  printf("%p->%p %d\n", old_root, new_root, change_value);
}

serial_dev_t screen = 0; 

LIBK_THREAD(_demo_thread){
	//serial_dev_t screen = fst6_get_screen_serial_interface(); 
	//fst6_key_mask_t keys = fst6_read_keys(); 
	static uint16_t sticks[6]; 
	
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
							m2_put_key(M2_KEY_DATA_UP); 
						else
							m2_put_key(M2_KEY_SELECT); 
						break; 
					case 9: 
						if(pressed & (1 << 14))
							m2_put_key(M2_KEY_DATA_DOWN); 
						else
							m2_put_key(M2_KEY_EXIT); 
						break; 
					case 16: 
						if(pressed & (1 << 14)){
							if(pressed & (1 << 15)) {
								m2_put_key(M2_KEY_DATA_UP); 
							}
							else {
								m2_put_key(M2_KEY_DATA_DOWN); 
							}
						} else {
							if(pressed & (1 << 15)) {
								m2_put_key(M2_KEY_NEXT); 
							}
							else {
								m2_put_key(M2_KEY_PREV); 
							}
						}
						break; 
				}
				//printf("Key: %d\n", c); 
			} else if(!(keys & (1 << c)) && (pressed & (1 << c))){ // released
				pressed &= ~(1 << c); 
			}
		}
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
int main(void){
	fst6_init(); 
	
	printf("SystemCoreClock: %d\n", (int)SystemCoreClock); 
	
	app.eeprom = fst6_get_storage_device(); 
	screen = fst6_get_screen_serial_interface(); 
	
	m2_tty_init(screen, &top_el_tlsm); 
	
	//m2_Init(&m2_root, m2_es_custom, m2_eh_4bs, m2_gh_tty);

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
