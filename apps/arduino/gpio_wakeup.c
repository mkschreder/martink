#include <kernel/mt.h>
#include <serial/serial.h>
#include <gpio/atmega_gpio.h>
#include <gpio/gpio.h>
#include <arch/soc.h>

static sem_t pcint_signal; 

static void pin_interrupt_irq(struct pcint_handler *self){
	sem_give_from_isr(&pcint_signal); 
}

static struct pcint_handler gpio_irq = {
	.pin = GPIO_INT0, 
	//.type = PCINT_EDGE_UP, 
	.handler = pin_interrupt_irq
}; 

static void pcint_thread(void *ptr){
	msleep(100); 
	for(;;){
		sem_take(&pcint_signal); 
		printk("PCINT! %d\r\n", gpio_read_pin(GPIO_INT0)); 
		cpu_powerdown(); 
	}
}

#include <radio/nrf24l01.h>
static void main_thread(void *ptr){
	static int tick = 0; 

	sem_init(&pcint_signal, 1, 0); 

	gpio_register_pcint(&gpio_irq); 
	gpio_configure(GPIO_INT0, GP_PULLUP); 
	gpio_enable_pcint(GPIO_INT0); 

	static struct nrf24l01 nrf; 
	struct spi_adapter *adapter = atmega_spi_get_adapter(); 
	nrf24l01_init(&nrf, adapter, GPIO_PB1, GPIO_PB0);  
	nrf24l01_powerdown(&nrf); 

	//sem_give(&pcint_signal); 
	for(;;){
		printk("TICK %d\r\n", tick++); 
		msleep(1000); 
	}
}

int main(void){
	thread_t thread, thread2; 
	
	thread_create(&thread, 200, 1, NULL, main_thread, NULL); 	
	thread_create(&thread2, 200, 1, NULL, pcint_thread, NULL); 	

	schedule_start(); 

	for(;;); 
	return 0; 
}

