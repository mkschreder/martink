#include <arch/soc.h>

static void _serial_fd_putc(int c, FILE *stream){
	serial_dev_t dev = fdev_get_udata(stream); 
	serial_putc(dev, c); 
}

static int _serial_fd_getc(FILE *stream){
	serial_dev_t dev = fdev_get_udata(stream); 
	return serial_getc(dev) & 0xff; 
}

uint16_t serial_printf(serial_dev_t port, const char *fmt, ...){
	FILE fd; 
	
	fdev_setup_stream(&fd, _serial_fd_putc, _serial_fd_getc, _FDEV_SETUP_RW); 
	fdev_set_udata(&fd, port); 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vfprintf(&fd, fmt, vl); 
	va_end(vl);
	return n; 
}
