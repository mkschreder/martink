#pragma once

struct device; 
struct device_driver {
	const char *name; 
	int (*probe)(struct device *dev); 
	int (*remove)(struct device *dev); 
	int (*suspend)(struct device *dev); 
}; 

struct device {
	struct bus_type *bus; 
	struct device_driver *driver; 
}; 

