#include <arch/soc.h>

#include "fd_serial.h"

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <memory.h>

void fd_serial_init(struct fd_serial *self, int in_fd, int out_fd){
	self->in_fd = in_fd; 
	self->out_fd = out_fd; 
}

static uint16_t _fd_serial_putc(serial_dev_t dev, uint8_t ch){
	struct fd_serial *self = container_of(dev, struct fd_serial, api); 
	int ret = write(self->out_fd, &ch, 1); 
	if(ret < 0) return SERIAL_BUFFER_FULL; 
	return 1; 
}

static uint16_t _fd_serial_getc(serial_dev_t dev) {
	struct fd_serial *self = container_of(dev, struct fd_serial, api); 
	uint8_t ch; 
	int ret = read(self->in_fd, &ch, 1); 
	if(ret < 0) return SERIAL_NO_DATA; 
	//printf("read: %d %c\n", ret, ch); 
	return ch; 
}

static size_t _fd_serial_putn(serial_dev_t dev, const uint8_t *data, size_t sz){
	struct fd_serial *self = container_of(dev, struct fd_serial, api); 
	size_t count = sz; 
	while(count){
		int ret = write(self->out_fd, data + (sz - count), count); 
		if(ret > 0) count -= ret; 
	}
	return sz; 
}

static size_t _fd_serial_getn(serial_dev_t dev, uint8_t *data, size_t sz){
	struct fd_serial *self = container_of(dev, struct fd_serial, api); 
	size_t count = sz; 
	while(count){
		int ret = read(self->in_fd, data + (sz - count), count); 
		if(ret > 0) count -= ret; 
	}
	return sz; 
}

static size_t _fd_serial_waiting(serial_dev_t self){
	(void)self; 
	return 0; 
}

static int16_t _fd_serial_begin(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

static int16_t _fd_serial_end(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

static struct serial_if _fd_serial_if = {
	.put = _fd_serial_putc,
	.get = _fd_serial_getc,
	.putn = _fd_serial_putn,
	.getn = _fd_serial_getn,
	.begin = _fd_serial_begin,
	.end = _fd_serial_end,
	.waiting = _fd_serial_waiting
}; 

serial_dev_t fd_serial_get_interface(struct fd_serial *self){
	self->api = &_fd_serial_if; 
	return &self->api; 
}

static int getkey(void) {
	int character;
	struct termios orig_term_attr;
	struct termios new_term_attr;

	/* set the terminal to raw mode */
	tcgetattr(fileno(stdin), &orig_term_attr);
	memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ECHO|ICANON);
	new_term_attr.c_cc[VTIME] = 0;
	new_term_attr.c_cc[VMIN] = 0;
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

	/* read a character from the stdin stream without blocking */
	/*   returns EOF (-1) if no character is available */
	character = fgetc(stdin);

	/* restore the original terminal attributes */
	tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

	return character;
}


static uint16_t _fd_stdin_getc(serial_dev_t dev) {
	(void)dev; 
	int ret = getkey(); 
	if(ret < 0) return SERIAL_NO_DATA; 
	return ret; 
}

static size_t _fd_stdin_getn(serial_dev_t dev, uint8_t *data, size_t sz){
	(void)dev; 
	size_t count = sz; 
	while(count){
		int ret = getkey(); 
		if(ret < 0) return sz - count; 
		data[sz - count] = ret; 
		count--; 
	}
	return sz; 
}

serial_dev_t stdio_get_serial_interface(void){
	static struct fd_serial console; 
	static struct serial_if _fd_stdio_if; 
	// create bridge for native stdin/stdout
	fd_serial_init(&console, STDIN_FILENO, STDOUT_FILENO); 
	
	// replace some methods in the interface
	memcpy(&_fd_stdio_if, &_fd_serial_if, sizeof(struct serial_if)); 
	_fd_stdio_if.get = _fd_stdin_getc; 
	_fd_stdio_if.getn = _fd_stdin_getn; 
	
	console.api = &_fd_stdio_if; 
	return &console.api; 
}
