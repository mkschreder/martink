/**
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include "vt100.h"
#include <arch/soc.h>

#define KEY_ESC 0x1b
#define KEY_DEL 0x7f
#define KEY_BELL 0x07

/*
#define VT100_SCREEN_WIDTH ili9340_width()
#define VT100_SCREEN_HEIGHT ili9340_height()
#define VT100_HEIGHT (VT100_SCREEN_HEIGHT / VT100_CHAR_HEIGHT)
#define VT100_WIDTH (VT100_SCREEN_WIDTH / VT100_CHAR_WIDTH)*/

#define STATE(NAME, TERM, EV, ARG) static void NAME(struct vt100 *TERM, uint8_t EV, uint16_t ARG)

// states 
enum {
	STATE_IDLE,
	STATE_ESCAPE,
	STATE_COMMAND
};

// events that are passed into states
enum {
	EV_CHAR = 1,
};


STATE(_st_idle, term, ev, arg);
STATE(_st_esc_sq_bracket, term, ev, arg);
STATE(_st_esc_question, term, ev, arg);
STATE(_st_esc_hash, term, ev, arg);
static void _vt100_putc(struct vt100 *t, uint8_t ch); 

static void _vt100_reset(struct vt100 *self){
	//self->screen_width = VT100_SCREEN_WIDTH;
  //self->screen_height = VT100_SCREEN_HEIGHT;
	tty_get_size(self->display, &self->screen_width, &self->screen_height); 
  
  //self->char_height = 8;
  //self->char_width = 6;
  self->back_color = 0x0000;
  self->front_color = 0xffff;
  self->cursor_x = self->cursor_y = self->saved_cursor_x = self->saved_cursor_y = 0;
  self->narg = 0;
  self->state = _st_idle;
  self->ret_state = 0;
  self->scroll_value = 0; 
  self->scroll_start_row = 0;
  self->scroll_end_row = self->screen_height; // outside of screen = whole screen scrollable
  self->flags.cursor_wrap = 1;
  self->flags.origin_mode = 0;
  
  /*
  ili9340_setFrontColor(self->front_color);
	ili9340_setBackColor(self->back_color);
	ili9340_setScrollMargins(0, 0); 
	ili9340_setScrollStart(0); */
	//self->display->set_scroll_region(self->display, 0, 0);
	//self->display->set_top_line(self->display, 0); 
}

static void _vt100_resetScroll(struct vt100 *self){
	self->scroll_start_row = 0;
	self->scroll_end_row = self->screen_height;
	self->scroll_value = 0;
	
	//self->display->set_scroll_region(self->display, 0, 0);
	//self->display->set_top_line(self->display, 0); 
}

/*
#define VT100_CURSOR_X(TERM) (TERM->cursor_x * TERM->char_width)

static uint16_t VT100_CURSOR_Y(struct vt100 *t){
	// if within the top or bottom margin areas then normal addressing
	if(t->cursor_y < t->scroll_start_row || t->cursor_y >= t->scroll_end_row){
		return t->cursor_y * VT100_CHAR_HEIGHT; 
	} else {
		// otherwise we are inside scroll area
		uint16_t scroll_height = t->scroll_end_row - t->scroll_start_row;
		uint16_t row = t->cursor_y + t->scroll_value; 
		if(t->cursor_y + t->scroll_value >= t->scroll_end_row)
			row -= scroll_height; 
		// if scroll_value == 0: y = t->cursor_y;
		// if scroll_value == 1 && scroll_start_row == 2 && scroll_end_row == 38:
		// 		y = t->cursor_y + scroll_value; 
		//uint16_t row = (t->cursor_y - t->scroll_start_row) % scroll_height; 
		//uint16_t skip = t->scroll_value - t->scroll_start_row; 
		//uint16_t row = t->cursor_y + skip;
		//uint16_t scroll_height = t->scroll_end_row - t->scroll_start_row; 
		//row = (row % scroll_height);// + t->scroll_start_row;
		return row * VT100_CHAR_HEIGHT; 
	}
}*/

static void _vt100_clearLines(struct vt100 *self, uint16_t start_line, uint16_t end_line){
	for(unsigned c = start_line; c <= end_line; c++){
		self->cursor_y = c; 
		self->cursor_x = 0; 
		for(unsigned j = 0; j < self->screen_width; j++){
			_vt100_putc(self, ' '); 
		}
		//uint16_t cy = t->cursor_y;
		//t->cursor_y = c;
		//tty_clear(t->display); 
		//ili9340_fillRect(0, VT100_CURSOR_Y(t), VT100_SCREEN_WIDTH, VT100_CHAR_HEIGHT, 0x0000);
		//t->cursor_y = cy;
	}
	self->cursor_x = 0; 
	self->cursor_y = start_line; 
	/*uint16_t start = ((start_line * t->char_height) + t->scroll) % VT100_SCREEN_HEIGHT;
	uint16_t h = (end_line - start_line) * VT100_CHAR_HEIGHT;
	ili9340_fillRect(0, start, VT100_SCREEN_WIDTH, h, 0x0000); */
}

// scrolls the scroll region up (lines > 0) or down (lines < 0)
static void _vt100_scroll(struct vt100 *t, int16_t lines){
	if(!lines) return;

	// get height of scroll area in rows
	uint16_t scroll_height = t->scroll_end_row - t->scroll_start_row; 
	// clearing of lines that we have scrolled up or down
	if(lines > 0){
		_vt100_clearLines(t, t->scroll_start_row, t->scroll_start_row+lines-1); 
		// update the scroll value (wraps around scroll_height)
		t->scroll_value = (t->scroll_value + lines) % scroll_height;
		// scrolling up so clear first line of scroll area
		//uint16_t y = (t->scroll_start_row + t->scroll_value) * VT100_CHAR_HEIGHT; 
		//ili9340_fillRect(0, y, VT100_SCREEN_WIDTH, lines * VT100_CHAR_HEIGHT, 0x0000);
	} else if(lines < 0){
		_vt100_clearLines(t, t->scroll_end_row - lines, t->scroll_end_row - 1); 
		// make sure that the value wraps down 
		t->scroll_value = (scroll_height + t->scroll_value + lines) % scroll_height; 
		// scrolling down - so clear last line of the scroll area
		//uint16_t y = (t->scroll_start_row + t->scroll_value) * VT100_CHAR_HEIGHT; 
		//ili9340_fillRect(0, y, VT100_SCREEN_WIDTH, lines * VT100_CHAR_HEIGHT, 0x0000);
	}
	//uint16_t scroll_start = (t->scroll_start_row + t->scroll_value) * VT100_CHAR_HEIGHT; 
	//t->display->set_top_line(t->display, scroll_start); 
	
	/*
	int16_t pixels = lines * VT100_CHAR_HEIGHT;
	uint16_t scroll_min = t->top_margin * VT100_CHAR_HEIGHT;
	uint16_t scroll_max = t->bottom_margin * VT100_CHAR_HEIGHT;

	// starting position must be between top and bottom margin
	// scroll_start == top margin - no scroll at all
	if(t->scroll >= scroll_min){
		// clear the top n lines
		ili9340_fillRect(0, t->scroll, VT100_SCREEN_WIDTH, pixels, 0x0000); 
		t->scroll += pixels;
	} else {
		ili9340_fillRect(0, scroll_min, VT100_SCREEN_WIDTH, pixels, 0x0000); 
		t->scroll = scroll_min + pixels;
	}
	t->scroll = t->scroll % VT100_SCREEN_HEIGHT; 
	ili9340_setScrollStart(t->scroll);*/
}

// moves the cursor relative to current cursor position and scrolls the screen
static void _vt100_move(struct vt100 *term, int16_t right_left, int16_t bottom_top){
	// calculate how many lines we need to move down or up if x movement goes outside screen
	int16_t new_x = right_left + term->cursor_x;
	int16_t width = term->screen_width; //(term->screen_width / VT100_CHAR_WIDTH); 
	if(new_x >= width){
		if(term->flags.cursor_wrap){
			bottom_top += new_x / width;
			term->cursor_x = new_x % width;
		} else {
			term->cursor_x = new_x;
		}
	} else if(new_x < 0){
		bottom_top += new_x / width - 1;
		term->cursor_x = width - (abs(new_x) % width) + 1; 
	} else {
		term->cursor_x = new_x;
	}

	if(bottom_top){
		uint16_t new_y = term->cursor_y + bottom_top;
		uint16_t to_scroll = 0;
		// bottom margin 39 marks last line as static on 40 line display
		// therefore, we would scroll when new cursor has moved to line 39
		// (or we could use new_y > VT100_HEIGHT here
		// NOTE: new_y >= term->scroll_end_row ## to_scroll = (new_y - term->scroll_end_row) +1
		if(new_y >= term->scroll_end_row){
			//scroll = new_y / VT100_HEIGHT;
			//term->cursor_y = VT100_HEIGHT;
			to_scroll = (new_y - term->scroll_end_row) + 1; 
			// place cursor back within the scroll region
			term->cursor_y = term->scroll_end_row - 1; //new_y - to_scroll; 
			//scroll = new_y - term->bottom_margin; 
			//term->cursor_y = term->bottom_margin; 
		} else if(new_y < term->scroll_start_row){
			to_scroll = (new_y - term->scroll_start_row); 
			term->cursor_y = term->scroll_start_row; //new_y - to_scroll; 
			//scroll = new_y / (term->bottom_margin - term->top_margin) - 1;
			//term->cursor_y = term->top_margin; 
		} else {
			// otherwise we move as normal inside the screen
			term->cursor_y = new_y;
		}
		_vt100_scroll(term, to_scroll);
	}
}

static void _vt100_drawCursor(struct vt100 *t){
	(void)(t); 
	//uint16_t x = t->cursor_x * t->char_width;
	//uint16_t y = t->cursor_y * t->char_height;

	//ili9340_fillRect(x, y, t->char_width, t->char_height, t->front_color); 
}

// sends the character to the display and updates cursor position
static void _vt100_putc(struct vt100 *t, uint8_t ch){
	/*if(ch < 0x20 || ch > 0x7e){
		static const char hex[] = "0123456789abcdef"; 
		_vt100_putc(t, '0'); 
		_vt100_putc(t, 'x'); 
		_vt100_putc(t, hex[((ch & 0xf0) >> 4)]);
		_vt100_putc(t, hex[(ch & 0x0f)]);
		return;
	}
	*/
	// calculate current cursor position in the display ram
	//uint16_t x = VT100_CURSOR_X(t);
	//uint16_t y = VT100_CURSOR_Y(t);
	
	tty_move_cursor(t->display, t->cursor_x, t->cursor_y); 
	tty_put(t->display, ch, t->front_color, t->back_color); 
	
	//t->display->draw_char(t->display, ch, x, y, t->front_color, t->back_color); 
	/*ili9340_setFrontColor(t->front_color);
	ili9340_setBackColor(t->back_color); 
	ili9340_drawChar(x, y, ch);*/

	// move cursor right
	_vt100_move(t, 1, 0); 
	_vt100_drawCursor(t); 
}


STATE(_st_command_arg, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			if(isdigit(arg)){ // a digit argument
				term->args[term->narg] = term->args[term->narg] * 10 + (arg - '0');
			} else if(arg == ';') { // separator
				term->narg++;
			} else { // no more arguments
				// go back to command state 
				term->narg++;
				if(term->ret_state){
					term->state = term->ret_state;
				}
				else {
					term->state = _st_idle;
				}
				// execute next state as well because we have already consumed a char!
				term->state(term, ev, arg);
			}
			break;
		}
	}
}

STATE(_st_esc_sq_bracket, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			if(isdigit(arg)){ // start of an argument
				term->ret_state = _st_esc_sq_bracket; 
				_st_command_arg(term, ev, arg);
				term->state = _st_command_arg;
			} else if(arg == ';'){ // arg separator. 
				// skip. And also stay in the command state
			} else { // otherwise we execute the command and go back to idle
				switch(arg){
					case 'A': {// move cursor up (cursor stops at top margin)
						unsigned n = (term->narg > 0)?term->args[0]:1;
						if(term->cursor_y < n) term->cursor_y = 0; 
						else term->cursor_y -= n;
						term->state = _st_idle; 
						break;
					} 
					case 'B': { // cursor down (cursor stops at bottom margin)
						unsigned n = (term->narg > 0)?term->args[0]:1;
						term->cursor_y += n;
						if(term->cursor_y > term->screen_height) term->cursor_y = term->screen_height; 
						term->state = _st_idle; 
						break;
					}
					case 'C': { // cursor right (cursor stops at right margin)
						unsigned n = (term->narg > 0)?term->args[0]:1;
						term->cursor_x += n;
						if(term->cursor_x > term->screen_width) term->cursor_x = term->screen_width;
						term->state = _st_idle; 
						break;
					}
					case 'D': { // cursor left
						unsigned n = (term->narg > 0)?term->args[0]:1;
						if(term->cursor_x < n) term->cursor_x = 0; 
						else term->cursor_x -= n;
						term->state = _st_idle; 
						break;
					}
					case 'f': 
					case 'H': { // move cursor to position (default 0;0)
						// cursor stops at respective margins
						term->cursor_x = (term->narg >= 1)?(term->args[1]-1):0; 
						term->cursor_y = (term->narg == 2)?(term->args[0]-1):0;
						if(term->flags.origin_mode) {
							term->cursor_y += term->scroll_start_row;
							if(term->cursor_y >= term->scroll_end_row){
								term->cursor_y = term->scroll_end_row - 1;
							}
						}
						if(term->cursor_x > term->screen_width) term->cursor_x = term->screen_width;
						if(term->cursor_y > term->screen_height) term->cursor_y = term->screen_height; 
						term->state = _st_idle; 
						break;
					}
					case 'J':{// clear screen from cursor up or down
						//uint16_t y = VT100_CURSOR_Y(term); 
						if(term->narg == 0 || (term->narg == 1 && term->args[0] == 0)){
							// clear down to the bottom of screen (including cursor)
							_vt100_clearLines(term, term->cursor_y, term->screen_height); 
						} else if(term->narg == 1 && term->args[0] == 1){
							// clear top of screen to current line (including cursor)
							_vt100_clearLines(term, 0, term->cursor_y); 
						} else if(term->narg == 1 && term->args[0] == 2){
							// clear whole screen
							_vt100_clearLines(term, 0, term->screen_height);
							// reset scroll value
							_vt100_resetScroll(term); 
						}
						term->state = _st_idle; 
						break;
					}
					case 'K':{// clear line from cursor right/left
						uint16_t cx = term->cursor_x; 
						uint16_t cy = term->cursor_y; 
						//uint16_t x = VT100_CURSOR_X(term);
						//uint16_t y = VT100_CURSOR_Y(term);
						//uint16_t w = term->screen_width / VT100_CHAR_WIDTH;
						//uint16_t h = term->screen_height / VT100_CHAR_HEIGHT;
						
						if(term->narg == 0 || (term->narg == 1 && term->args[0] == 0)){
							// clear to end of line (to \n or to edge?)
							// including cursor
							for(unsigned c = term->cursor_x; c < term->screen_width; c++) _vt100_putc(term, ' '); 
							//term->display->draw_fill_rect(term->display, x, y, w - x, VT100_CHAR_HEIGHT, term->back_color); 
							//ili9340_fillRect(x, y, VT100_SCREEN_WIDTH - x, VT100_CHAR_HEIGHT, term->back_color);
						} else if(term->narg == 1 && term->args[0] == 1){
							term->cursor_x = 0; 
							for(unsigned c = term->cursor_x; c < cx; c++) _vt100_putc(term, ' '); 
							// clear from left to current cursor position
							//term->display->draw_fill_rect(term->display, 0, y, x + w, VT100_CHAR_HEIGHT, term->back_color);
						} else if(term->narg == 1 && term->args[0] == 2){
							term->cursor_x = 0; 
							for(unsigned c = term->cursor_x; c < term->screen_width; c++) _vt100_putc(term, ' '); 
							// clear whole current line
							//term->display->draw_fill_rect(term->display, 0, y, w, VT100_CHAR_HEIGHT, term->back_color);
						}
						term->cursor_x = cx; 
						term->cursor_y = cy; 
						term->state = _st_idle; 
						break;
					}
					
					case 'L': // insert lines (args[0] = number of lines)
					case 'M': // delete lines (args[0] = number of lines)
						term->state = _st_idle;
						break; 
					case 'P': {// delete characters args[0] or 1 in front of cursor
						// TODO: this needs to correctly delete n chars
						int n = ((term->narg > 0)?term->args[0]:1);
						_vt100_move(term, -n, 0);
						for(int c = 0; c < n; c++){
							_vt100_putc(term, ' ');
						}
						term->state = _st_idle;
						break;
					}
					case 'c':{ // query device code
						//term->send_response("\e[?1;0c"); 
						term->state = _st_idle; 
						break; 
					}
					case 'x': {
						term->state = _st_idle;
						break;
					}
					case 's':{// save cursor pos
						term->saved_cursor_x = term->cursor_x;
						term->saved_cursor_y = term->cursor_y;
						term->state = _st_idle; 
						break;
					}
					case 'u':{// restore cursor pos
						term->cursor_x = term->saved_cursor_x;
						term->cursor_y = term->saved_cursor_y; 
						//_vt100_moveCursor(term, term->saved_cursor_x, term->saved_cursor_y);
						term->state = _st_idle; 
						break;
					}
					case 'h':
					case 'l': {
						term->state = _st_idle;
						break;
					}
					
					case 'g': {
						term->state = _st_idle;
						break;
					}
					case 'm': { // sets colors. Accepts up to 3 args
						// [m means reset the colors to default
						if(!term->narg){
							term->front_color = 0xffff;
							term->back_color = 0x0000;
						}
						while(term->narg){
							term->narg--; 
							int n = term->args[term->narg];
							static const uint16_t colors[] = {
								0x0000, // black
								0xf800, // red
								0x0780, // green
								0xfe00, // yellow
								0x001f, // blue
								0xf81f, // magenta
								0x07ff, // cyan
								0xffff // white
							};
							if(n == 0){ // all attributes off
								term->front_color = 0xffff;
								term->back_color = 0x0000;
								
							}
							if(n >= 30 && n < 38){ // fg colors
								term->front_color = colors[n-30]; 
							} else if(n >= 40 && n < 48){
								term->back_color = colors[n-40]; 
							}
						}
						term->state = _st_idle; 
						break;
					}
					
					case '@': // Insert Characters          
						term->state = _st_idle;
						break; 
					case 'r': // Set scroll region (top and bottom margins)
						// the top value is first row of scroll region
						// the bottom value is the first row of static region after scroll
						if(term->narg == 2 && term->args[0] < term->args[1]){
							// [1;40r means scroll region between 8 and 312
							// bottom margin is 320 - (40 - 1) * 8 = 8 pix
							term->scroll_start_row = term->args[0] - 1;
							term->scroll_end_row = term->args[1] - 1; 
							//uint16_t top_margin = term->scroll_start_row * VT100_CHAR_HEIGHT;
							//uint16_t bottom_margin = term->screen_height -
							//	(term->scroll_end_row * VT100_CHAR_HEIGHT); 
							//term->display->set_scroll_region(term->display, top_margin, bottom_margin);
							//ili9340_setScrollStart(0); // reset scroll 
						} else {
							_vt100_resetScroll(term); 
						}
						term->state = _st_idle; 
						break;  
					case 'i': // Printing  
					case 'y': // self test modes..
					case '=':{ // argument follows... 
						//term->state = _st_screen_mode;
						term->state = _st_idle; 
						break; 
					}
					case '?': // '[?' escape mode
						term->state = _st_esc_question;
						break; 
					default: { // unknown sequence
						
						term->state = _st_idle;
						break;
					}
				}
				//term->state = _st_idle;
			} // else
			break;
		}
		default: { // switch (ev)
			// for all other events restore normal mode
			term->state = _st_idle; 
		}
	}
}

STATE(_st_esc_question, term, ev, arg){
	// DEC mode commands
	switch(ev){
		case EV_CHAR: {
			if(isdigit(arg)){ // start of an argument
				term->ret_state = _st_esc_question; 
				_st_command_arg(term, ev, arg);
				term->state = _st_command_arg;
			} else if(arg == ';'){ // arg separator. 
				// skip. And also stay in the command state
			} else {
				switch(arg) {
					case 'l': 
						// dec mode: OFF (arg[0] = function)
					case 'h': {
						// dec mode: ON (arg[0] = function)
						switch(term->args[0]){
							case 1: { // cursor keys mode
								// h = esc 0 A for cursor up
								// l = cursor keys send ansi commands
								break;
							}
							case 2: { // ansi / vt52
								// h = ansi mode
								// l = vt52 mode
								break;
							}
							case 3: {
								// h = 132 chars per line
								// l = 80 chars per line
								break;
							}
							case 4: {
								// h = smooth scroll
								// l = jump scroll
								break;
							}
							case 5: {
								// h = black on white bg
								// l = white on black bg
								break;
							}
							case 6: {
								// h = cursor relative to scroll region
								// l = cursor independent of scroll region
								term->flags.origin_mode = (arg == 'h')?1:0; 
								break;
							}
							case 7: {
								// h = new line after last column
								// l = cursor stays at the end of line
								term->flags.cursor_wrap = (arg == 'h')?1:0; 
								break;
							}
							case 8: {
								// h = keys will auto repeat
								// l = keys do not auto repeat when held down
								break;
							}
							case 9: {
								// h = display interlaced
								// l = display not interlaced
								break;
							}
							// 10-38 - all quite DEC speciffic commands so omitted here
						}
						term->state = _st_idle;
						break; 
					}
					case 'i': /* Printing */  
					case 'n': /* Request printer status */
					default:  
						term->state = _st_idle; 
						break;
				}
				term->state = _st_idle;
			}
		}
	}
}

STATE(_st_esc_left_br, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			switch(arg) {  
				case 'A':  
				case 'B':  
					// translation map command?
				case '0':  
				case 'O':
					// another translation map command?
					term->state = _st_idle;
					break;
				default:
					term->state = _st_idle;
			}
			//term->state = _st_idle;
		}
	}
}

STATE(_st_esc_right_br, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			switch(arg) {  
				case 'A':  
				case 'B':  
					// translation map command?
				case '0':  
				case 'O':
					// another translation map command?
					term->state = _st_idle;
					break;
				default:
					term->state = _st_idle;
			}
			//term->state = _st_idle;
		}
	}
}

STATE(_st_esc_hash, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			switch(arg) {  
				case '8': {
					// self test: fill the screen with 'E'
					
					term->state = _st_idle;
					break;
				}
				default:
					term->state = _st_idle;
			}
		}
	}
}

STATE(_st_escape, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			#define CLEAR_ARGS \
				{ term->narg = 0;\
				for(int c = 0; c < VT100_MAX_COMMAND_ARGS; c++)\
					term->args[c] = 0; }\
			
			switch(arg){
				case '[': { // command
					// prepare command state and switch to it
					CLEAR_ARGS; 
					term->state = _st_esc_sq_bracket;
					break;
				}
				case '(': /* ESC ( */  
					CLEAR_ARGS;
					term->state = _st_esc_left_br;
					break; 
				case ')': /* ESC ) */  
					CLEAR_ARGS;
					term->state = _st_esc_right_br;
					break;  
				case '#': // ESC # 
					CLEAR_ARGS;
					term->state = _st_esc_hash;
					break;  
				case 'P': //ESC P (DCS, Device Control String)
					term->state = _st_idle; 
					break;
				case 'D': // moves cursor down one line and scrolls if necessary
					// move cursor down one line and scroll window if at bottom line
					_vt100_move(term, 0, 1); 
					term->state = _st_idle;
					break; 
				case 'M': // Cursor up
					// move cursor up one line and scroll window if at top line
					_vt100_move(term, 0, -1); 
					term->state = _st_idle;
					break; 
				case 'E': // next line
					// same as '\r\n'
					_vt100_move(term, 0, 1);
					term->cursor_x = 0; 
					term->state = _st_idle;
					break;  
				case '7': // Save attributes and cursor position  
				case 's':  
					term->saved_cursor_x = term->cursor_x;
					term->saved_cursor_y = term->cursor_y;
					term->state = _st_idle;
					break;  
				case '8': // Restore them  
				case 'u': 
					term->cursor_x = term->saved_cursor_x;
					term->cursor_y = term->saved_cursor_y; 
					term->state = _st_idle;
					break; 
				case '=': // Keypad into applications mode 
					term->state = _st_idle;
					break; 
				case '>': // Keypad into numeric mode   
					term->state = _st_idle;
					break;  
				case 'Z': // Report terminal type 
					// vt 100 response
					//term->send_response("\033[?1;0c");  
					// unknown terminal     
						//out("\033[?c");
					term->state = _st_idle;
					break;    
				case 'c': // Reset terminal to initial state 
					_vt100_reset(term);
					term->state = _st_idle;
					break;  
				case 'H': // Set tab in current position 
				case 'N': // G2 character set for next character only  
				case 'O': // G3 "               "     
				case '<': // Exit vt52 mode
					// ignore
					term->state = _st_idle;
					break; 
				case KEY_ESC: { // marks start of next escape sequence
					// stay in escape state
					break;
				}
				default: { // unknown sequence - return to normal mode
					term->state = _st_idle;
					break;
				}
			}
			#undef CLEAR_ARGS
			break;
		}
		default: {
			// for all other events restore normal mode
			term->state = _st_idle; 
		}
	}
}

STATE(_st_idle, term, ev, arg){
	switch(ev){
		case EV_CHAR: {
			switch(arg){
				
				case 5: // AnswerBack for vt100's  
					//term->send_response("X"); // should send SCCS_ID?
					break;  
				case '\n': { // new line
					_vt100_move(term, 0, 1);
					term->cursor_x = 0; 
					//_vt100_moveCursor(term, 0, term->cursor_y + 1);
					// do scrolling here! 
					break;
				}
				case '\r': { // carrage return (0x0d)
					term->cursor_x = 0; 
					//_vt100_move(term, 0, 1);
					//_vt100_moveCursor(term, 0, term->cursor_y); 
					break;
				}
				case '\b': { // backspace 0x08
					_vt100_move(term, -1, 0); 
					// backspace does not delete the character! Only moves cursor!
					//ili9340_drawChar(term->cursor_x * term->char_width,
					//	term->cursor_y * term->char_height, ' ');
					break;
				}
				case KEY_DEL: { // del - delete character under cursor
					// Problem: with current implementation, we can't move the rest of line
					// to the left as is the proper behavior of the delete character
					// fill the current position with background color
					_vt100_putc(term, ' ');
					_vt100_move(term, -1, 0);
					//_vt100_clearChar(term, term->cursor_x, term->cursor_y); 
					break;
				}
				case '\t': { // tab
					// tab fills characters on the line until we reach a multiple of tab_stop
					int tab_stop = 4;
					int to_put = tab_stop - (term->cursor_x % tab_stop); 
					while(to_put--) _vt100_putc(term, ' ');
					break;
				}
				case KEY_BELL: { // bell is sent by bash for ex. when doing tab completion
					// sound the speaker bell?
					// skip
					break; 
				}
				case KEY_ESC: {// escape
					term->state = _st_escape;
					break;
				}
				default: {
					_vt100_putc(term, arg);
					break;
				}
			}
			break;
		}
		default: {}
	}
}

void vt100_init(struct vt100 *self, tty_dev_t display){
	self->display = display; 
  //self->send_response = send_response; 
	_vt100_reset(self); 
}

void vt100_putc(struct vt100 *self, uint8_t c){
	/*char *buffer = 0; 
	switch(c){
		case KEY_UP:         buffer="\e[A";    break;
		case KEY_DOWN:       buffer="\e[B";    break;
		case KEY_RIGHT:      buffer="\e[C";    break;
		case KEY_LEFT:       buffer="\e[D";    break;
		case KEY_BACKSPACE:  buffer="\b";      break;
		case KEY_IC:         buffer="\e[2~";   break;
		case KEY_DC:         buffer="\e[3~";   break;
		case KEY_HOME:       buffer="\e[7~";   break;
		case KEY_END:        buffer="\e[8~";   break;
		case KEY_PPAGE:      buffer="\e[5~";   break;
		case KEY_NPAGE:      buffer="\e[6~";   break;
		case KEY_SUSPEND:    buffer="\x1A";    break;      // ctrl-z
		case KEY_F(1):       buffer="\e[[A";   break;
		case KEY_F(2):       buffer="\e[[B";   break;
		case KEY_F(3):       buffer="\e[[C";   break;
		case KEY_F(4):       buffer="\e[[D";   break;
		case KEY_F(5):       buffer="\e[[E";   break;
		case KEY_F(6):       buffer="\e[17~";  break;
		case KEY_F(7):       buffer="\e[18~";  break;
		case KEY_F(8):       buffer="\e[19~";  break;
		case KEY_F(9):       buffer="\e[20~";  break;
		case KEY_F(10):      buffer="\e[21~";  break;
	}
	if(buffer){
		while(*buffer){
			self->state(&term, EV_CHAR, *buffer++);
		}
	} else {
		self->state(&term, EV_CHAR, 0x0000 | c);
	}*/
	self->state(self, EV_CHAR, c);
}


void vt100_puts(struct vt100 *term, const char *str){
	while(*str){
		vt100_putc(term, *str++);
	}
}

/*******************
* SERIAL INTERFACE
********************/

static uint16_t _vt100_serial_putc(serial_dev_t self, uint8_t ch){
	struct vt100 *dev = container_of(self, struct vt100, serial); 
	vt100_putc(dev, ch); 
	return 0; 
}

static uint16_t _vt100_serial_getc(serial_dev_t self) {
	(void)(self); 
	return SERIAL_NO_DATA; 
}

static size_t _vt100_serial_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_vt100_serial_putc(self, *data++); 
	}
	return size; 
}

static size_t _vt100_serial_getn(serial_dev_t self, uint8_t *data, size_t sz){
	(void)(self); 
	(void)(data); 
	(void)(sz); 
	return 0; 
}

static size_t _vt100_serial_waiting(serial_dev_t self){
	(void)(self); 
	return 0; 
}

static int16_t _vt100_serial_begin(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

static int16_t _vt100_serial_end(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

serial_dev_t vt100_get_serial_interface(struct vt100 *self){
	static struct serial_if _if;
	_if = (struct serial_if) {
		.put = _vt100_serial_putc,
		.get = _vt100_serial_getc,
		.putn = _vt100_serial_putn,
		.getn = _vt100_serial_getn,
		.begin = _vt100_serial_begin,
		.end = _vt100_serial_end,
		.waiting = _vt100_serial_waiting
	}; 
	
	self->serial = &_if;
	return &self->serial; 
}
