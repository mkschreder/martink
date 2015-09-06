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
	// the vt100 size is determined by the size of the underlying framebuffer
	tty_get_size(self->display, &self->screen_width, &self->screen_height); 

	self->back_color = 0x0000;
	self->front_color = 0xffff;
	self->cursor_x = self->cursor_y = self->saved_cursor_x = self->saved_cursor_y = 0;
	self->narg = 0;
	self->state = _st_idle;
	self->ret_state = 0;
	self->scroll_value = 0; 
	self->scroll_start_row = 0;
	self->scroll_end_row = self->screen_height; // outside of screen = whole screen scrollable
	self->flags = VT100_FLAG_CURSOR_WRAP; 
}

/*
static void _vt100_resetScroll(struct vt100 *self){
	self->scroll_start_row = 0;
	self->scroll_end_row = self->screen_height;
	self->scroll_value = 0;
}
*/

static void _vt100_clearLines(struct vt100 *self, uint16_t start_line, uint16_t end_line){
	if(start_line > end_line) return; 
	while(start_line <= end_line){
		for(unsigned j = 0; j < self->screen_width; j++){
			tty_move_cursor(self->display, j, start_line); 
			tty_put(self->display, ' ', self->front_color, self->back_color); 
		}
		start_line ++; 
	}
}

/*
// scrolls the scroll region up (lines > 0) or down (lines < 0)
static void _vt100_scroll(struct vt100 *t, int16_t lines){
	// scroll is not working when we can not read the display. Only possibly on displays that have
	// built in scrolling functionality. 
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
	
}*/

// moves the cursor relative to current cursor position and scrolls the screen
static void _vt100_move(struct vt100 *term, int16_t right_left, int16_t bottom_top){
	// calculate how many lines we need to move down or up if x movement goes outside screen
	int16_t new_x = right_left + term->cursor_x;
	int16_t new_y = bottom_top + term->cursor_y; 
	int16_t width = term->screen_width; //(term->screen_width / VT100_CHAR_WIDTH); 
	
	if(new_x < 0) new_x = 0; 
	if(new_y < 0) new_y = 0; 
	
	if(!(term->flags & VT100_FLAG_CURSOR_WRAP)){
		term->cursor_x = ((uint16_t)new_x > term->screen_width)?(term->screen_width):(uint16_t)new_x; 
		term->cursor_y = ((uint16_t)new_y > term->screen_height)?term->screen_height:(uint16_t)new_y; 
		return; 
	}
	// TODO: simplify this
	if(new_x >= width){
		if(term->flags & VT100_FLAG_CURSOR_WRAP){
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
		uint16_t to_scroll = 0;
		// bottom margin 39 marks last line as static on 40 line display
		// therefore, we would scroll when new cursor has moved to line 39
		// (or we could use new_y > VT100_HEIGHT here
		// NOTE: new_y >= term->scroll_end_row ## to_scroll = (new_y - term->scroll_end_row) +1
		if((uint16_t)new_y >= term->scroll_end_row){
			//scroll = new_y / VT100_HEIGHT;
			//term->cursor_y = VT100_HEIGHT;
			to_scroll = (new_y - term->scroll_end_row) + 1; 
			// place cursor back within the scroll region
			term->cursor_y = term->scroll_end_row - 1; //new_y - to_scroll; 
			//scroll = new_y - term->bottom_margin; 
			//term->cursor_y = term->bottom_margin; 
		} else if((uint16_t)new_y < term->scroll_start_row){
			to_scroll = (new_y - term->scroll_start_row); 
			term->cursor_y = term->scroll_start_row; //new_y - to_scroll; 
			//scroll = new_y / (term->bottom_margin - term->top_margin) - 1;
			//term->cursor_y = term->top_margin; 
		} else {
			// otherwise we move as normal inside the screen
			term->cursor_y = new_y;
		}
		
		(void)to_scroll; 
		//_vt100_scroll(term, to_scroll);
	}
}


// sends the character to the display and updates cursor position
static void _vt100_putc(struct vt100 *t, uint8_t ch){
	// send character to text display
	tty_move_cursor(t->display, t->cursor_x, t->cursor_y); 
	tty_put(t->display, ch, t->front_color, t->back_color); 
	
	// update cursor position
	_vt100_move(t, 1, 0); 
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
						if(term->flags & VT100_FLAG_ORIGIN_MODE) {
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
							//_vt100_resetScroll(term); 
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
						//printf("Set color: %x %x\n", term->front_color, term->back_color); 
						
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
						} else {
							//_vt100_resetScroll(term); 
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
								term->flags |= (arg == 'h')?VT100_FLAG_ORIGIN_MODE:0; 
								break;
							}
							case 7: {
								// h = new line after last column
								// l = cursor stays at the end of line
								term->flags |= (arg == 'h')?VT100_FLAG_CURSOR_WRAP:0; 
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

// todo: should be const 
static struct serial_if _vt100_if = {
	.put = _vt100_serial_putc,
	.get = _vt100_serial_getc,
	.putn = _vt100_serial_putn,
	.getn = _vt100_serial_getn,
	.begin = _vt100_serial_begin,
	.end = _vt100_serial_end,
	.waiting = _vt100_serial_waiting
}; 

serial_dev_t vt100_get_serial_interface(struct vt100 *self){
	self->serial = &_vt100_if;
	return &self->serial; 
}
