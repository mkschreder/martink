#include "adc.h"
#include <kernel/types.h>

static LIST_HEAD(_adc_devices); 

void adc_register_device(struct adc_device *dev){
	list_add_tail(&_adc_devices, &dev->list); 
}

struct adc_device *adc_get_device(uint8_t number){
	// TODO: slow method
	struct adc_device *dev = NULL, *adc; 
	list_for_each_entry(adc, &_adc_devices, list){
		if(!number) { dev = adc; break; }
		number--; 
	}
	return dev; 
}
