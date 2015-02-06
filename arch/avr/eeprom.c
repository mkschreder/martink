#include <arch/soc.h>
#include "eeprom.h"

//#ifdef CONFIG_HAVE_EEPROM
static size_t	_ee_write(memory_dev_t self, size_t address, const uint8_t *data, size_t max_sz){
	eeprom_update_block(data, (uint8_t*)address, max_sz); 
	return max_sz; 
}

static size_t	_ee_read(memory_dev_t self, size_t address, uint8_t *data, size_t max_sz){
	eeprom_read_block(data, (uint8_t*)address, max_sz); 
	return max_sz; 
}
	
void 		_ee_readInfo(uint16_t *block_size, size_t *device_size){
	*block_size = 1; 
	*device_size = 0; 
}

memory_dev_t eeprom_get_memory_interface(void){
	static struct memory_if _i; 
	static struct memory_if *i = 0; 
	if(!i){
		_i.read = _ee_read; 
		_i.write = _ee_write; 
		_i.readInfo = _ee_readInfo; 
		i = &_i; 
	}
	return &i; 
}
//#endif
