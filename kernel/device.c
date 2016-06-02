
#include "device.h"
#include "driver.h"

#include <stdlib.h>

#if 0
static LIST_HEAD(_devices); 

void device_register(struct device *self){
	INIT_LIST_HEAD(&self->list); 
	list_add_tail(&self->list, &_devices);  
}

struct device *device_find(struct list_head *list, const char *path){
	int size = strlen(path) + 1; 
	char p[10] = {0}; 
	strncpy(p, path, 10); 
	char *pn = p; 
	char *j = p; 
	while(*j) { if(*j == ':') { pn = j+1; *j = 0; } j++; }
	int n = atoi(pn); 
	struct device *dev; 
	list_for_each_entry(dev, list, list){
		if(strcmp(dev->driver->name, p) == 0){
			if(n == 0) return dev; 
			n--; 
		}
	}
	return 0; 
}

void device_foreach(void (*func)(struct device *dev, void *p), void *p){
	struct device *dev, *tmp; 
	list_for_each_entry_safe(dev, tmp, &_devices, list){
		func(dev, p); 
	}
}
#endif
