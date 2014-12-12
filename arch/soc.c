#include "soc.h"

const struct d_soc cpu = {
	//.uart0 = DEV_CHAR_DEVICE(CONFIG_UART0_NAME)
	#ifdef CONFIG_HAVE_UART0
	.uart0 = DEV_CHAR_DEVICE(CONFIG_UART0_NAME), 
	#endif
	#ifdef CONFIG_HAVE_UART1
	//.uart1 = DEV_CHAR_DEVICE(CONFIG_UART1_NAME),
	#endif
	#ifdef CONFIG_HAVE_UART2
	//.uart2 = DEV_CHAR_DEVICE(CONFIG_UART2_NAME),
	#endif
	#ifdef CONFIG_HAVE_UART3
	//.uart3 = DEV_CHAR_DEVICE(CONFIG_UART3_NAME),
	#endif
	#ifdef CONFIG_HAVE_SPI0
	.spi0 = DEV_SPI_DEVICE(CONFIG_SPI0_NAME),
	#endif
	#ifdef CONFIG_HAVE_SPI1
	//.spi1 = DEV_SPI_DEVICE(CONFIG_SPI1_NAME)
	#endif
	#ifdef CONFIG_HAVE_TWI0
	//soc->twi0 = BLOCK_API(CONFIG_TWI0_NAME); 
	#endif
	#ifdef CONFIG_HAVE_TWI1
	//soc->twi1 = BLOCK_API(CONFIG_TWI1_NAME); 
	#endif
};

const struct d_soc *main_cpu(void)  {
	return &cpu; 
}

