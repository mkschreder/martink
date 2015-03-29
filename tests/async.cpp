#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include <kernel/thread/async.h>
#include "../arch/native/time.c"
#include "../arch/time.c"
#include "../kernel/thread.c"
#include "../kernel/cpp/thread.hpp"

#include <string.h>

class Test {
public:
	Test(const char *str) {
		printf("Test::%s::ctor\n", str); 
		this->str = str; 
	}
	~Test(){
		printf("Test::%s::dtor\n", str); 
	}
	const char *str; 
}; 

class MyClass {
public:
	ASYNC_MEMBER_PROTOTYPE(int, task, const char *str); 
	ASYNC_MEMBER_PROTOTYPE(int, task2); 
	ASYNC_MEMBER_PROTOTYPE(int, task3); 
	ASYNC_MEMBER_PROTOTYPE(int, ticker_task); 
	//struct async_task task, task2, task3, ticker_task;
	timestamp_t time, ticker_time;  
}; 

ASYNC_MEMBER(int, MyClass, task, const char *str){
	ASYNC_BEGIN(); 
	printf("task start: %s\n", str); 
	
	static int c = 0; 
	for(c = 0; c < 10; c++){
		this->time = timestamp_from_now_us(100000); 
		printf("task: waiting %s\n", str); 
		AWAIT(timestamp_expired(this->time)); 
		ASYNC_YIELD(); 
	}
	printf("task end: %s\n", str); 
	ASYNC_END(1); 
}

ASYNC_MEMBER(int, MyClass, task2){
	ASYNC_BEGIN(); 
	printf("task2: waiting..\n"); 
	int result = AWAIT_MEMBER_TASK(int, this->task, "from 2"); 
	printf("task2: completed with result: %d\n", result); 
	ASYNC_END(0); 
}

ASYNC_MEMBER(int, MyClass, task3){
	ASYNC_BEGIN(); 
	printf("task3: waiting..\n"); 
	AWAIT_MEMBER_TASK(int, this->task, "from 3"); 
	printf("task3: completed\n"); 
	ASYNC_END(0); 
}

ASYNC_MEMBER(int, MyClass, ticker_task){
	ASYNC_BEGIN(); 
	this->ticker_time = timestamp_from_now_us(100000); 
	AWAIT(timestamp_expired(this->ticker_time));
	printf(".\n"); 
	ASYNC_END(0); 
}

class Thread; 

class Thread {
public:
	Thread(); 
	~Thread(); 
	
	virtual async_return_t ThreadProc(int *ret, struct async_task *parent) = 0; 
	struct async_process *GetProcessPtr(){ return &this->process; }
	
	struct async_task __ThreadProc__; 
	struct async_process process; 
};

ASYNC_PROCESS(Thread_Process){
	class Thread *thr = cpp_container_of(self, &Thread::process);  
	int ret = 0; 
	
	ASYNC_BEGIN(); 
	
	ret = AWAIT_MEMBER_TASK(int, thr->ThreadProc); 
	
	ASYNC_END(ret); 
}


Thread::Thread(){
	printf("Thread()\n"); 
	ASYNC_INIT(&this->__ThreadProc__); 
	ASYNC_PROCESS_INIT(&this->process, Thread_Process); 
	//ASYNC_QUEUE_WORK(ASYNC_GLOBAL_QUEUE, &this->process, Thread_Process); 
}

Thread::~Thread(){
	
}

class MyThread : public Thread {
public:
	virtual async_return_t ThreadProc(int *ret, struct async_task *parent); 
	
	MyClass obj; 
}; 

ASYNC_QUEUE(queue); 

static struct async_process bottom_half; 
ASYNC_PROCESS(bottom_half_proc){
	ASYNC_BEGIN(); 
	printf("Bottom half\n"); 
	ASYNC_END(0); 
}


static int running = 1;
ASYNC_MEMBER(int, MyThread, ThreadProc){
	ASYNC_BEGIN(); 
	while(1){
		obj.ticker_task(0, 0); 
		obj.task3(0, 0); 
		if(obj.task2(0, 0) == ASYNC_ENDED) {
			ASYNC_QUEUE_WORK(&queue, &bottom_half); 
			running = 0; 
			break; 
		}
		ASYNC_YIELD(); 
	}
	ASYNC_END(0); 
}

ASYNC_PROCESS(main_process_proc){
	ASYNC_BEGIN(); 
	while(ASYNC_RUN_QUEUE(&queue)){
		//printf("Main process\n"); 
	}
	ASYNC_END(0); 
}

struct parp {
	struct async_process process; 
	timestamp_t time; 
}; 

ASYNC_PROCESS(parprocess){
	struct parp *p = container_of(self, struct parp, process); 
	ASYNC_BEGIN(); 
	while(running){
		printf("Parprocess %p\n", self); 
		AWAIT_DELAY(p->time, rand() % 500000); 
	}
	ASYNC_END(0); 
}

int main(void){
	printf("STARTING..\n"); 
	
	MyThread thr; 
	struct async_process main_process; 
	struct parp parp1, parp2, parp3; 
	
	ASYNC_PROCESS_INIT(&parp1.process, parprocess); 
	ASYNC_PROCESS_INIT(&parp2.process, parprocess); 
	ASYNC_PROCESS_INIT(&parp3.process, parprocess); 
	
	ASYNC_PROCESS_INIT(&bottom_half, bottom_half_proc); 
	ASYNC_PROCESS_INIT(&main_process, main_process_proc); 
	
	ASYNC_QUEUE_WORK(&queue, &parp1.process); 
	ASYNC_QUEUE_WORK(&queue, &parp2.process); 
	ASYNC_QUEUE_WORK(&queue, &parp3.process); 
	
	ASYNC_QUEUE_WORK(&queue, &bottom_half); 
	ASYNC_QUEUE_WORK(&queue, thr.GetProcessPtr()); 
	ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, &main_process); 
	
	while(ASYNC_RUN_QUEUE(&ASYNC_GLOBAL_QUEUE));
	
	//while(ASYNC_RUN_QUEUE(&queue)); 
	
	printf("Done\n"); 
}
