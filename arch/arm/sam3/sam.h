#pragma once


/* Define attribute */
#if defined (  __GNUC__  ) /* GCC CS3 */
    #define WEAK __attribute__ ((weak))
#elif defined ( __ICCARM__ ) /* IAR Ewarm 5.41+ */
    #define WEAK __weak
#endif

/* Define NO_INIT attribute */
#if defined (  __GNUC__  )
    #define NO_INIT
#elif defined ( __ICCARM__ )
    #define NO_INIT __no_init
#endif

#include "CMSIS/sam/sam.h"

#include "adc.h"
#include "rtc.h"
#include "ssc.h"
#include "twi.h"
#include "wdt.h"
#include "gpbr.h"
#include "pmc.h"
#include "rtt.h"
#include "tc.h"
#include "udp.h"
#include "usart.h"
#include "dacc.h"
#include "interrupt_sam_nvic.h"
#include "pwmc.h"
#include "timetick.h"
//#include "udphs.h"
#include "USB_device.h"
#include "efc.h"
#include "pio.h"
#include "rstc.h"
#include "spi.h"
#include "USB_host.h"

#if (SAM3XA_SERIES)
#include "can.h"
#include "emac.h"
#include "trng.h"
#include "uotghs_device.h"
#include "uotghs_host.h"
#endif
