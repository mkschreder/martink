#pragma once

#include "autoconf.h"

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// these should be moved somewhere. This is the way to do configurable
// c interfaces using macros. It is used by all generic interface headers. 
#define PFCALL2(P, F, args...) __##P##_##F##__(args)
#define PFCALL(P, F, args...) PFCALL2(P, F, args)


#include "time.h"
#include "uart.h"
#include "twi.h"

#ifdef CONFIG_ATMEGA328P
#include "avr/m328p.h"
#endif

#ifdef CONFIG_ARM
#include "arm/arm.h"
#endif

#ifdef CONFIG_NATIVE
#include "native/native.h"
#endif

#ifdef __cplusplus
}
#endif
