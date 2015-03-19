#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include <kernel/thread/async.h>
#include "../arch/native/time.c"
#include "../arch/time.c"

#include <string.h>

struct ticker {
	timestamp_t time; 
}; 

struct myobject {
	struct async_task task, task2, task3, ticker_task;
	timestamp_t time;  
	struct ticker ticker; 
}; 

typedef struct myobject myobject_t; 

struct lock_state {
	struct async_task *saved; 
	struct async_task **user_thread; 
}; 

static inline void _user_thread_dtor(struct lock_state *lock){
	printf("Cleanup lock\n"); 
	
}

ASYNC(myobject_t, task, const char *str){
	ASYNC_BEGIN(); 
	printf("task start: %s\n", str); 
	static int c = 0; 
	for(c = 0; c < 10; c++){
		self->time = timestamp_from_now_us(100000); 
		printf("task: waiting %s\n", str); 
		AWAIT(timestamp_expired(self->time)); 
		ASYNC_YIELD(); 
	}
	printf("task end: %s\n", str); 
	ASYNC_END(); 
}

ASYNC(myobject_t, task2){
	ASYNC_BEGIN(); 
	printf("task2: waiting..\n"); 
	AWAIT_TASK(myobject_t, task, self, "from 2"); 
	printf("task2: completed\n"); 
	ASYNC_END(); 
}

ASYNC(myobject_t, task3){
	ASYNC_BEGIN(); 
	printf("task3: waiting..\n"); 
	AWAIT_TASK(myobject_t, task, self, "from 3"); 
	printf("task3: completed\n"); 
	ASYNC_END(); 
}

ASYNC(myobject_t, ticker_task){
	ASYNC_BEGIN(); 
	self->ticker.time = timestamp_from_now_us(100000); 
	AWAIT(timestamp_expired(self->ticker.time));
	printf(".\n"); 
	ASYNC_END(); 
}

void myobject_init(struct myobject *self){
	ASYNC_INIT(&self->task); 
	ASYNC_INIT(&self->task2); 
	ASYNC_INIT(&self->task3); 
	ASYNC_INIT(&self->ticker_task); 
}

int main(void){
	printf("STARTING..\n"); 
	struct myobject obj; 
	myobject_init(&obj); 
	
	while(1){
		/*
		ASYNC(test, int a){
			ASYNC_BEGIN(
			AWAIT_TASK(task2, &obj); 
		*/
		ASYNC_INVOKE_ONCE(myobject_t, ticker_task, 0, &obj); 
		ASYNC_INVOKE_ONCE(myobject_t, task3, 0, &obj); 
		if(ASYNC_INVOKE_ONCE(myobject_t, task2, 0, &obj) == ASYNC_ENDED) break; 
	}
	
	printf("Done\n"); 
}
