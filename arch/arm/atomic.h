#pragma once 

extern int __cscount; 

#define sei() __asm volatile ("cpsie i"); 
#define cli() __asm volatile ("cpsid i"); 

static inline uint32_t _cli(void) __attribute__((always_inline, unused));
static inline void _sei(const uint32_t *primask) __attribute__((always_inline,
                                                                 unused));

static void _sei( const uint32_t *_new ){
	__asm volatile(
		"	msr basepri, %0	" :: "r" ( *_new )
	);
}


static inline uint32_t _cli( void ){
	uint32_t _orig, _new;
	__asm volatile
	(
		"	mrs %0, basepri											\n" \
		"	mov %1, %2												\n"	\
		"	msr basepri, %1											\n" \
		"	isb														\n" \
		"	dsb														\n" \
		:"=r" (_orig), "=r" (_new) : "i" ( 1 )
	);

	return _orig;
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
