#pragma once 

extern int __cscount; 

#define sei() __ASM volatile ("cpsie i"); 
#define cli() __ASM volatile ("cpsid i"); 

static inline uint32_t _cli(void) __attribute__((always_inline, unused));
static inline void _sei(const uint32_t *primask) __attribute__((always_inline,
                                                                 unused));

static inline uint32_t
_cli(void)
{
	uint32_t  primask = 0;
	__ASM volatile("mrs %0, primask\n" : "=r" (primask)::);
	__disable_irq();
	return primask;
}

static inline void
_sei(const uint32_t *primask)
{
	if (*primask == 0) {
		__enable_irq();
	}
}

#define ATOMIC_BLOCK(type) for ( type, __ToDo = 1; __ToDo ; __ToDo = 0 )
#define NONATOMIC_BLOCK(type) for ( type, __ToDo = __iSeiRetVal(); \
	                          __ToDo ;  __ToDo = 0 )

#define ATOMIC_RESTORESTATE uint32_t primask_save \
	__attribute__((__cleanup__(_sei))) = _cli()

/*
#define ATOMIC_FORCEON uint8_t sreg_save \
	__attribute__((__cleanup__(_sei))) = 0

#define NONATOMIC_RESTORESTATE uint8_t primask_save \
	__attribute__((__cleanup__(_sei))) = __iReadPrimask()

#define NONATOMIC_FORCEOFF uint8_t sreg_save \
	__attribute__((__cleanup__(__iCliParam))) = 0
*/
