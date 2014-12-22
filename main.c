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

#include "kernel.h"

extern void app_init(void); 
extern void app_process_events(void); 

int main(void){
	/*struct ssd1306 ssd; 
	sei(); 
	
	uart0_init(38400); 
	uart0_puts("INIT..\n"); 
	
	time_init(); 
	i2c_init(); 
	
	ssd1306_init(&ssd); 
	
	ssd1306_clear(&ssd); 
	//ssd1306_set_region(&brd->ssd, 0, 0, 128, 64); 
	//ssd1306_fill(&brd->ssd, RGBA(0, 0, 0, 0), 128*64); 
	
	char buffer[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; 
	while(1){
		ssd1306_draw(&ssd, 0, 0, (struct fb_image){
			.w = 8, 
			.h = 8, 
			.data = buffer, 
			.format = FB_PIXEL_FORMAT_1BIT
		}); 
		ssd1306_set_region(&ssd, 0, 0, 5, 50); 
		ssd1306_fill(&ssd, RGBA(0, 0, 0, 0), 5*50); 
		ssd1306_set_region(&ssd, 5, 0, 50, 50); 
		ssd1306_fill(&ssd, RGBA(255, 255, 255, 255), 50*50); 
		_delay_ms(500); 
		ssd1306_clear(&ssd); 
		ssd1306_set_region(&ssd, 50, 0, 5, 50); 
		ssd1306_fill(&ssd, RGBA(0, 0, 0, 0), 5*50); 
		ssd1306_set_region(&ssd, 0, 0, 50, 50); 
		ssd1306_fill(&ssd, RGBA(255, 255, 255, 255), 50*50); 
		_delay_ms(500); 
		//ssd1306_puts(&brd->ssd, "Hello World!\n", 10); 
	}
	
	return; */
	//brd_init(); 
	
	app_init(); 
	
	while(1){
		//brd_process_events(); 
		app_process_events(); 
	}
}

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);}
void __cxa_guard_release (__guard *g) {*(char *)g = 1;}
void __cxa_guard_abort (__guard *g) {}
void __cxa_pure_virtual(void) {}
