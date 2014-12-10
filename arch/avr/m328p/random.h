#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void prng_init(volatile uint8_t *inport, volatile uint8_t *ddrport, uint8_t rndpin); 

#ifdef __cplusplus
}
#endif
