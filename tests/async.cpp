#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include "../kernel/thread/async.c"
#include "../arch/native/time.c"
#include "../arch/time.c"
#include "../kernel/thread.c"
#include "../kernel/cpp/thread.hpp"

#include <string.h>

class MyClass {
public:
	ASYNC_MEMBER_PROTOTYPE(int, task, const char *str); 
	ASYNC_MEMBER_PROTOTYPE(int, task2); 
	ASYNC_MEMBER_PROTOTYPE(int, task3); 
	ASYNC_MEMBER_PROTOTYPE(int, ticker_task); 
	timestamp_t time, ticker_time;  
private:
	ASYNC_MEMBER_DATA(task); 
	ASYNC_MEMBER_DATA(task2); 
	ASYNC_MEMBER_DATA(task3); 
	ASYNC_MEMBER_DATA(ticker_task); 
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
	printf("ticker_task\n"); 
	ASYNC_END(0); 
}

class MyThread : public AsyncProcess {
public:
	virtual ASYNC_MEMBER_PROTOTYPE(int, ThreadProc); 
	
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
	while(ASYNC_RUN_PARALLEL(&queue)){
		// NOTE: must yield here in order to return control to the top level loop
		ASYNC_YIELD(); 
	}
	ASYNC_END(0); 
}

struct parp {
	struct async_process process; 
	timestamp_t time; 
}; 

ASYNC_PROCESS(parprocess){
	struct parp *p = container_of(__self, struct parp, process); 
	ASYNC_BEGIN(); 
	while(running){
		printf("Parprocess %p\n", __self); 
		AWAIT_DELAY(p->time, rand() % 500000); 
	}
	ASYNC_END(0); 
}

class MySubProcess: public AsyncProcess {
public:
	MySubProcess(){
		printf("MySubProcess()\n"); 
	}
	virtual ~MySubProcess(){
		printf("~MySubProcess()\n"); 
	}
	virtual ASYNC_MEMBER_PROTOTYPE(int, ThreadProc) override;
private:
	timestamp_t time; 
}; 

ASYNC_MEMBER(int, MySubProcess, ThreadProc){
	ASYNC_BEGIN(); 
	printf("MySubProcess::ThreadProc(): enter\n"); 
	AWAIT_DELAY(time, 100000); 
	printf("MySubProcess::ThreadProc(): done!\n"); 
	ASYNC_END(0); 
}

class MyProcess : public AsyncProcess {
public: 
	MyProcess(){
		printf("Created myprocess\n"); 
	}
	virtual ~MyProcess(){
		printf("Deleted my process\n"); 
	}
	virtual ASYNC_MEMBER_PROTOTYPE(int, ThreadProc) override; 
private:
	ASYNC_MEMBER_PROTOTYPE(int, print, const char *str); 
	ASYNC_MEMBER_DATA(print); 
	timestamp_t time; 
	int count; 
	MySubProcess *subproc; 
}; 

ASYNC_MEMBER(int, MyProcess, print, const char *str){
	ASYNC_BEGIN(); 
	// ok this is not really async
	AWAIT_DELAY(time, 1); 
	int ret = printf("%s", str); 
	AWAIT_DELAY(time, 1); 
	ASYNC_END(ret); 
}

ASYNC_MEMBER(int, MyProcess, ThreadProc){
	ASYNC_BEGIN(); 
	AWAIT_DELAY(time, 500000); 
	
	AWAIT_MEMBER_TASK(int, print, "MyProcess: async print\n"); 
	
	for(count = 0; count < 5; count++){
		subproc = new MySubProcess(); 
		
		ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, subproc); 
		
		printf("Waiting for process to complete\n"); 
		AWAIT(subproc->Exited()); 
		printf("Process completed\n"); 
		
		AWAIT_DELAY(time, 500000); 
		
		// this is normally dangerous since we always need to make 
		// sure that the process has exited in order not to have it leave
		// resource in undefined state!
		delete subproc; 
	}
	ASYNC_END(0); 
}


struct mystruct {
	// this is data - must be same name as the method above. 
	struct async_task method_name; 
	struct async_process process; 
	// some other struct data 
	const char *str; 
}; 
// typedef because only single words can be used in C macros
typedef struct mystruct mystruct_t; 
ASYNC(int, mystruct_t, method_name){
	// async methods always get __self parameter which points to 
	// structure of type mystruct_t (in this case) 
	struct mystruct *self = __self; 
	ASYNC_BEGIN(); 
	// use the struct data
	printf("%s\n", self->str); 
	ASYNC_END(0); 
}

ASYNC_PROCESS(mystruct_process){
	// retreive the struct itself 
	struct mystruct *self = container_of(__self, struct mystruct, process); 
	ASYNC_BEGIN(); 
	AWAIT_TASK(int, mystruct_t, method_name, self); 
	ASYNC_END(0); 
}

void mystruct_init(struct mystruct *self, const char *str){
	ASYNC_INIT(&self->method_name); 
	ASYNC_PROCESS_INIT(&self->process, mystruct_process); 
	ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, &self->process); 
	self->str = str; 
}

int main(void){
	printf("STARTING..\n"); 
	
	MyThread thr; 
	MyProcess proc; 
	
	struct async_process main_process; 
	struct parp parp1, parp2, parp3; 
	struct mystruct my_struct; 
	
	// this will queue a process automatically 
	mystruct_init(&my_struct, "Hello World!\n"); 
	
	ASYNC_PROCESS_INIT(&parp1.process, parprocess); 
	ASYNC_PROCESS_INIT(&parp2.process, parprocess); 
	ASYNC_PROCESS_INIT(&parp3.process, parprocess); 
	
	ASYNC_PROCESS_INIT(&bottom_half, bottom_half_proc); 
	ASYNC_PROCESS_INIT(&main_process, main_process_proc); 
	
	ASYNC_QUEUE_WORK(&queue, &proc); 
	ASYNC_QUEUE_WORK(&queue, &parp1.process); 
	ASYNC_QUEUE_WORK(&queue, &parp2.process); 
	ASYNC_QUEUE_WORK(&queue, &parp3.process); 
	
	ASYNC_QUEUE_WORK(&queue, &bottom_half); 
	ASYNC_QUEUE_WORK(&queue, thr.GetProcessPtr()); 
	
	ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, &main_process); 
	
	while(ASYNC_RUN_PARALLEL(&ASYNC_GLOBAL_QUEUE));
	
	//delete sub; 
	
	printf("Done\n"); 
}
