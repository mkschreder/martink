#include <arch/soc.h>
#include "eeprom.h"

#if 0
struct avr_eeprom {
	size_t address; 
	struct device dev; 
}; 

static size_t	_ee_write(struct device *dev, const uint8_t *data, size_t max_sz){
	(void)(self); 
	eeprom_update_block(data, (uint8_t*)address, max_sz); 
	return max_sz; 
}

static size_t	_ee_read(struct device *dev, uint8_t *data, size_t max_sz){
	(void)(self); 
	eeprom_read_block(data, (uint8_t*)address, max_sz); 
	return max_sz; 
}
	
static void _ee_readInfo(uint16_t *block_size, size_t *device_size){
	*block_size = 1; 
	*device_size = 0; 
}
#endif
