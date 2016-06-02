#define CONFIG_NATIVE 1

#include <termios.h>
#include <string.h>
#include <signal.h>

#include "../arch/soc.h"
#include "../arch/native/time.c"
#include "../arch/time.c"

#include "../disp/text.c"
#include "../arch/native/fd_serial.c"
#include "../gui/m2_tty.c"
#include "../kernel/cbuf.c"
#include "../arch/soc.c"
#include "../kernel/thread.c"
#include "../tty/vt100.c"

#include "../lib/m2gui/m2align.c"
#include "../lib/m2gui/m2.c"
#include "../lib/m2gui/m2dfs.c"
#include "../lib/m2gui/m2draw.c"
#include "../lib/m2gui/m2eh2bd.c"
#include "../lib/m2gui/m2eh2bs.c"
#include "../lib/m2gui/m2eh4bd.c"
#include "../lib/m2gui/m2eh4bs.c"
#include "../lib/m2gui/m2eh6bs.c"
#include "../lib/m2gui/m2ehts.c"
#include "../lib/m2gui/m2el2lmenu.c"
#include "../lib/m2gui/m2elalign.c"
#include "../lib/m2gui/m2elbutton.c"
#include "../lib/m2gui/m2elbuttonptr.c"
#include "../lib/m2gui/m2elcombo.c"
#include "../lib/m2gui/m2elcombofn.c"
#include "../lib/m2gui/m2elcomboptr.c"
#include "../lib/m2gui/m2elfnfmt.c"
#include "../lib/m2gui/m2elhide.c"
#include "../lib/m2gui/m2elinfo.c"
#include "../lib/m2gui/m2ellabel.c"
#include "../lib/m2gui/m2ellabelfn.c"
#include "../lib/m2gui/m2ellabelp.c"
#include "../lib/m2gui/m2ellabelptr.c"
#include "../lib/m2gui/m2ellistbase.c"
#include "../lib/m2gui/m2ellistcalc.c"
#include "../lib/m2gui/m2ellistgrid.c"
#include "../lib/m2gui/m2ellisth.c"
#include "../lib/m2gui/m2ellistv.c"
#include "../lib/m2gui/m2ellistxy.c"
#include "../lib/m2gui/m2elradio.c"
#include "../lib/m2gui/m2elroot.c"
#include "../lib/m2gui/m2els8base.c"
#include "../lib/m2gui/m2els8num.c"
#include "../lib/m2gui/m2elsb.c"
#include "../lib/m2gui/m2elsetval.c"
#include "../lib/m2gui/m2elslbase.c"
#include "../lib/m2gui/m2elspace.c"
#include "../lib/m2gui/m2elstr.c"
#include "../lib/m2gui/m2elstrlist.c"
#include "../lib/m2gui/m2elstrptr.c"
#include "../lib/m2gui/m2elstrup.c"
#include "../lib/m2gui/m2elsubutl.c"
#include "../lib/m2gui/m2eltext.c"
#include "../lib/m2gui/m2eltoggle.c"
#include "../lib/m2gui/m2eltsk.c"
#include "../lib/m2gui/m2elu32.c"
#include "../lib/m2gui/m2elu8base.c"
#include "../lib/m2gui/m2elu8hs.c"
#include "../lib/m2gui/m2elu8num.c"
#include "../lib/m2gui/m2elxbm.c"
#include "../lib/m2gui/m2fnarg.c"
#include "../lib/m2gui/m2gfx.c"
#include "../lib/m2gui/m2gfxutl.c"
#include "../lib/m2gui/m2ghdummy.c"
#include "../lib/m2gui/m2key.c"
#include "../lib/m2gui/m2msg.c"
#include "../lib/m2gui/m2navdatadn.c"
#include "../lib/m2gui/m2navdataup.c"
#include "../lib/m2gui/m2navfirst.c"
#include "../lib/m2gui/m2navinit.c"
#include "../lib/m2gui/m2navlast.c"
#include "../lib/m2gui/m2navnext.c"
#include "../lib/m2gui/m2navprev.c"
#include "../lib/m2gui/m2navroot.c"
#include "../lib/m2gui/m2navupdn.c"
#include "../lib/m2gui/m2navutl.c"
#include "../lib/m2gui/m2null.c"
#include "../lib/m2gui/m2obj.c"
#include "../lib/m2gui/m2opt.c"
#include "../lib/m2gui/m2pin.c"
#include "../lib/m2gui/m2qk.c"
#include "../lib/m2gui/m2rom.c"
#include "../lib/m2gui/m2usrnext.c"
#include "../lib/m2gui/m2usrprev.c"
#include "../lib/m2gui/m2usrupdn.c"
#include "../lib/m2gui/m2utls8d.c"
#include "../lib/m2gui/m2utlstrdat.c"
#include "../lib/m2gui/m2utlu8d.c"
#include "../lib/m2gui/mas.c"
#include "../lib/m2gui/mas_sim.c"

#include "../examples/fst6-gui/gui.c"


#define SCREEN_WIDTH 21
#define SCREEN_HEIGHT 8

struct m2_tty tty; 
struct vt100 vt; // vt 100 interpreter
struct text_buffer disp; // text buffer that vt100 will write to 
serial_dev_t console; 

void handle_input(void){
	int16_t key = 0; 
	while((key = serial_getc(console)) != SERIAL_NO_DATA){
		switch(key){
			case 10: 
				m2_tty_put_key(&tty, M2_KEY_SELECT); 
				break; 
			case 'q': 
				m2_tty_put_key(&tty, M2_KEY_EXIT); 
				break; 
			case 's': 
				m2_tty_put_key(&tty, M2_KEY_DATA_DOWN); 
			case 'w': 
				m2_tty_put_key(&tty, M2_KEY_DATA_UP); 
				break; 
			case 66: 
				m2_tty_put_key(&tty, M2_KEY_NEXT); 
				break; 
			case 65: 
				m2_tty_put_key(&tty, M2_KEY_PREV); 
				break; 
		}
	}
}

LIBK_THREAD(update){
	PT_BEGIN(pt); 
	while(1){
		handle_input();
		// dump the vt100 text buffer to the console
		text_buffer_render(&disp, console); 
		PT_YIELD(pt); 
	}
	PT_END(pt); 
}

void sigint_handler(void){
	// reset the terminal and return to home before exiting
	serial_printf(console, "\x1b" "c\x1b[2J\x1b[H"); 
	exit(0); 
}

int main(void){
	uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT]; 
	
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigaction(SIGINT, &act, NULL);
	
	// initialize a text screen
	text_buffer_init(&disp, buffer, SCREEN_WIDTH, SCREEN_HEIGHT); 
	
	// provide the screen as target to the vt100 emulator
	vt100_init(&vt, text_buffer_get_interface(&disp)); 
	
	// initialize the m2 gui and pass a serial interface to the vt100 
	m2_tty_init(&tty, vt100_get_serial_interface(&vt), SCREEN_WIDTH, SCREEN_HEIGHT, &top_el_tlsm); 
	
	// get a serial wrapper around the native stdio
	console = stdio_get_serial_interface(); 
	
	// clear screen
	serial_printf(console, "\x1b[2J\x1b[12h"); 
	
	libk_run(); 
	
	return 0; 
}
