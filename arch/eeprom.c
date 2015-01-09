#include "eeprom.h"

static size_t	_ee_write(memory_dev_t self, size_t address, const uint8_t *data, size_t max_sz){
	#ifdef eeprom_update_block
	eeprom_update_block(data, (uint8_t*)address, max_sz); 
	return max_sz; 
	#endif
	return 0; 
}

static size_t	_ee_read(memory_dev_t self, size_t address, uint8_t *data, size_t max_sz){
	#ifdef eeprom_read_block
	eeprom_read_block(data, (uint8_t*)address, max_sz); 
	return max_sz; 
	#endif
	return 0; 
}
	
void 		_ee_readInfo(uint16_t *block_size, size_t *device_size){
	*block_size = 1; 
	*device_size = 0; 
}

memory_dev_t eeprom_get_memory_interface(void){
	static struct memory_if i; 
	if(!i.read){
		i.read = _ee_read; 
		i.write = _ee_write; 
		i.readInfo = _ee_readInfo; 
	}
	return (memory_dev_t)&i; 
}
