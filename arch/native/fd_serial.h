/*
 Code for interfacing standard unix file descriptors to serial interface 
 handles for use inside libk device drivers. 
*/

#pragma once

struct fd_serial {
	int in_fd, out_fd; // file descriptor
	struct serial_if *api; // serial device api
}; 

void fd_serial_init(struct fd_serial *self, int in_fd, int out_fd); 
serial_dev_t fd_serial_get_interface(struct fd_serial *self); 

// helper for getting serial interface to stdio
serial_dev_t stdio_get_serial_interface(void); 
