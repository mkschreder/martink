#include "openpilot_cc3d.h"

void cc3d_init(void){
	
}

void cc3d_process_events(void){
	
}

fc_board_t cc3d_get_fc_quad_interface(void){
	static fc_board_t _brd; 
	static fc_board_t *brd = &_brd; 
	return &_brd; 
}
