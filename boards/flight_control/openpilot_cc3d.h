#pragma once

#include "../interface.h"

#define MINCOMMAND 1000
#define MAXCOMMAND 1700

#ifdef __cplusplus
extern "C" {
#endif

void cc3d_init(void); 
void cc3d_process_events(void);

fc_board_t cc3d_get_fc_quad_interface(void);

#ifdef __cplusplus
}
#endif

#define fc_init() cc3d_init()
#define fc_process_events() cc3d_process_events()
#define fc_interface() cc3d_get_fc_quad_interface()
#define fc_led_on() gpio_set(0); 
#define fc_led_off() gpio_clear(0); 
