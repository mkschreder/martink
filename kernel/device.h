#pragma once

/*
typedef enum {
	DEV_SERIAL
} devops_type_t; 

struct device_ops_meta {}; 
	
struct device_ops {
	devops_type_t type; 
	struct device_ops_meta *meta; 
	struct device_ops *next; 
}; 

struct device_meta {}; 

struct device_info {
	char name[7]; 
	void (*init)(struct device_meta *meta); 
	struct device_ops *ops; 
	volatile const struct device_meta *meta; 
}; 

#define DEVICE_INFO(name) volatile static const struct device_info _dev_##name __attribute__ ((section (".device_info"))) __attribute__ ((unused)) 
#define DEVICE_META(type, name) volatile type _meta_##name __attribute__ ((section (".device_meta"))) __attribute__ ((unused))

#define SECTION_DEVICE_META __attribute__ ((section (".device_meta"))) __attribute__ ((used))
*/
