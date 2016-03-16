#pragma once

#include "list.h"

#if 0
struct device {
	struct list_head list; 
	struct device_driver *driver; 
}; 

struct device *device_find(struct list_head *list, const char *path); 
void device_register(struct device *dev); 
void device_foreach(void (*func)(struct device *dev, void *p), void *p); 

#endif
