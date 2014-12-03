#pragma once

#include "autoconf.h"

#ifdef CONFIG_ATMEGA328P
#include "m328p.h"
#endif

#ifdef CONFIG_NATIVE
#include "native/native.h"
#endif
