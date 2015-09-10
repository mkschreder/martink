#include <kernel.h>
#include <arch/soc.h>

struct print_hello {
	struct libk_device device; 
	const char *name; 
	uint32_t delay; 
}; 

//static async_return_t print_hello_clock(int *ret, struct async_task *task, struct async_process *process){
DEVICE_CLOCK(print_hello_clock){
	struct libk_device *device = DEVICE_CLOCK_GET_DEVICE(); 
	struct print_hello *self = container_of(device, struct print_hello, device); 
	ASYNC_BEGIN(); 
	while(1){
		DEBUG("Hello from %s\n", self->name); 
		DEVICE_DELAY(self->delay); 
	}
	ASYNC_END(0); 
}

static void print_hello_init(struct print_hello *self, uint32_t delay, const char *name){
	self->name = name; 
	self->delay = delay; 
	libk_register_device(&self->device, DEVICE_CLOCK_PTR(print_hello_clock), "print-hello"); 
}

static void __init module_init(void){
	static struct print_hello device; 
	static struct print_hello device2; 
	DEBUG("print-hello: init\n"); 
	print_hello_init(&device, 800000L, "hello1"); 
	print_hello_init(&device2, 400000L, "hello2"); 
}
