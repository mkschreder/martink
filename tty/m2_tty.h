#pragma once

#include <lib/m2gui/m2.h>

#ifdef __cplusplus
extern "C" {
#endif

void m2_tty_init(serial_dev_t _screen, m2_rom_void_p element); 
void m2_put_key(uint8_t key); 

#ifdef __cplusplus
}
#endif
