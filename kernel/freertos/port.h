/* Copyright (c) 2016 Martin K. Schröder */

#include "../../config.h"

#ifdef CONFIG_ARCH_LINUX
#include "portable/linux/portmacro.h"
#elif CONFIG_ARCH_AVR
#include "portable/ATMega323/portmacro.h"
#endif
