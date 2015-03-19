#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include <kernel/thread/async.h>
#include "../arch/native/time.c"
#include "../arch/time.c"

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
	ASYNC_DECL(task, const char *str); 
	ASYNC_DECL(task2); 
	ASYNC_DECL(task3); 
	ASYNC_DECL(ticker_task); 
	//struct async_task task, task2, task3, ticker_task;
	timestamp_t time, ticker_time;  
}; 

ASYNC(MyClass, task, const char *str){
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
	ASYNC_END(); 
}

ASYNC(MyClass, task2){
	ASYNC_BEGIN(); 
	printf("task2: waiting..\n"); 
	AWAIT_TASK(this->task, "from 2"); 
	printf("task2: completed\n"); 
	ASYNC_END(); 
}

ASYNC(MyClass, task3){
	ASYNC_BEGIN(); 
	printf("task3: waiting..\n"); 
	AWAIT_TASK(this->task, "from 3"); 
	printf("task3: completed\n"); 
	ASYNC_END(); 
}

ASYNC(MyClass, ticker_task){
	ASYNC_BEGIN(); 
	this->ticker_time = timestamp_from_now_us(100000); 
	AWAIT(timestamp_expired(this->ticker_time));
	printf(".\n"); 
	ASYNC_END(); 
}

int main(void){
	printf("STARTING..\n"); 
	MyClass obj; 
	
	while(1){
		/*
		ASYNC(test, int a){
			ASYNC_BEGIN(
			AWAIT_TASK(task2, &obj); 
		*/
		obj.ticker_task(0); 
		obj.task3(0); 
		if(obj.task2(0) == ASYNC_ENDED) break; 
	}
	
	printf("Done\n"); 
}
