/*
Interface to the application. Must be in a separate file because we
want to be able to test the gui using SDL without the application at all. 
*/

#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <gui/m2_fb.h>
#include "gui.hpp"
#include "gui_impl.c"

void gui_init(fbuf_dev_t fb){
	m2_fb_init(fb, &top_el_tlsm);
	//m2_tty_init(screen, &top_el_tlsm); 
}

struct gui_data* gui_get_data(void){
	return &model; 
}
