#include <arch/soc.h>
#include <kernel/list.h>
#include "serial.h"

LIST_HEAD(_serial_drivers); 

int serial_add_driver(struct serial_driver *driver){
	INIT_LIST_HEAD(&driver->list); 
	list_add_tail(&_serial_drivers, &driver->list); 
	return 0; 
}

int serial_remove_driver(struct serial_driver *driver){
	list_del_init(&driver->list); 
	return 0; 
}


