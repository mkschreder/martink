#include "adc.h"
#include <kernel/types.h>

static LIST_HEAD(_adc_devices); 

void adc_register_device(struct adc_device *dev){
	list_add_tail(&dev->list, &_adc_devices); 
}

struct adc_device *adc_get_device(uint8_t number){
	return list_get_entry(&_adc_devices, number, struct adc_device, list); 
}
