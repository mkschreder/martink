#ifndef HCSR04_H
#define HCSR04_H

#define HCSR04_MEAS_FAIL 0xFFFF

uint16_t hcsr04_get_distance_in_cm(void); 
void hcsr04_init(void); 

#endif
