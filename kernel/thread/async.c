#include <arch/soc.h>
#include "async.h"

#define ASYNC_DEBUG(...) {}
//{printf(__VA_ARGS__);}

static struct async_process *__current_process = 0; 
struct async_queue ASYNC_GLOBAL_QUEUE;  

void async_queue_init(struct async_queue *self){
	INIT_LIST_HEAD(&self->list); 
}

static void __attribute__((constructor)) _init_global_queue(void){
	//ASYNC_DEBUG("async: init\n"); 
	async_queue_init(&ASYNC_GLOBAL_QUEUE); 
}

void async_process_init(struct async_process *self, ASYNC_PTR(int, async_process_t, func), const char *name){
	(void)name; 
	ASYNC_INIT(&self->task); 
	INIT_LIST_HEAD(&self->list); 
	self->ASYNC_NAME(int, async_process_t, proc) = ASYNC_NAME(int, async_process_t, func); 
	self->name = name; 
	self->sleep_until = 0; 
	ASYNC_DEBUG("Proc: %p\n", self->ASYNC_NAME(int, async_process_t, proc)); 
}

void async_queue_process(async_queue_t *queue, struct async_process *self){
	if(self->ASYNC_NAME(int, async_process_t, proc) == 0) {
		ASYNC_DEBUG("NULL proc!\n"); return; // prevent invalid proc pointers..
	}
	if(self->list.prev != self->list.next) {
		ASYNC_DEBUG("ALREADY ADDED!\n"); 
		return; // prevent adding to queue twice
	}
	ASYNC_DEBUG("Self: %p %s\n", self->ASYNC_NAME(int, async_process_t, proc), self->name); 
	list_add_tail(&self->list, &queue->list); 
}


uint8_t async_queue_run_series(async_queue_t *queue){
	int ret = 0; 
	
	if(list_empty(&queue->list)) return 0; 
	struct async_process *p = list_first_entry(&queue->list, struct async_process, list); 
	
	__current_process = p; 
	if((p->ASYNC_NAME(int, async_process_t, proc)(&ret, 0, p)) == ASYNC_ENDED){
		// if the thread has exited then remove it from the list
		ASYNC_DEBUG("Removing %s from task list\n", p->name); 
		list_del_init(queue->list.next); 
	}
	__current_process = 0; 
	
	return 1; 
}

uint8_t async_queue_run_parallel(async_queue_t *queue){
	uint8_t active = 0; 
	struct list_head *ptr, *n; 
	if(list_empty(&queue->list)) return 0;
	timestamp_t min_sleep = 0x0fffffff; 
	list_for_each_safe(ptr, n, &queue->list){
		struct async_process *p = container_of(ptr, struct async_process, list); 
		int ret = 0; 
		
		__current_process = p; 
		if((p->ASYNC_NAME(int, async_process_t, proc)(&ret, 0, p)) == ASYNC_ENDED){
			// if the thread has exited then remove it from the list
			ASYNC_DEBUG("Removing %s from task list\n", p->name); 
			list_del_init(ptr); 
		}
		//if(p->sleep_until < queue->sleep_until && !timestamp_expired(p->sleep_until)) queue->sleep_until = p->sleep_until; 
		//else queue->sleep_until = timestamp_now(); 
		timestamp_t s = p->sleep_until - timestamp_now(); 
		if(s > 0 && s < min_sleep) min_sleep = s; 
		__current_process = 0; 
		//DEBUG("p %s: %d\n", p->name, p->sleep_until - timestamp_now()); 
		
		active++; 
	}
	//DEBUG("min sleep: %lu\n", min_sleep); 
	queue->sleep_until = timestamp_from_now_us(min_sleep); 
	return active; 
}
