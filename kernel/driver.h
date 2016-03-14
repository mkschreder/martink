#pragma once

struct device_driver {
	struct list_head list; 
	const char *name; 
	void (*probe)(void); 
}; 

#define module_driver(__driver, __register, __unregister, ...) \
static int __init __driver##_device_init(void){ \
	return __register(&(__driver), ##__VA_ARGS__);\
}\
static int __exit __driver##_device_exit(void){ \
	return __unregister(&(__driver), ##__VA_ARGS__); \
}
