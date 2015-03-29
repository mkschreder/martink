#pragma once

#include "lc.h"
#include "../list.h"

#define ASYNC_WAITING 0
#define ASYNC_ENDED   3

typedef char async_return_t; 
typedef uint8_t async_mutex_t; 

#define ASYNC_MUTEX_INIT(mutex, max) (mutex) = max
#define ASYNC_MUTEX_LOCK(mutex) do { AWAIT(mutex); --(mutex); } while(0)
#define ASYNC_MUTEX_UNLOCK(mutex) (mutex)++

#define AWAIT_DELAY(var, time_us) do { var = timestamp_from_now_us(time_us); AWAIT(timestamp_expired(var)); } while(0)

#define ASYNC_NAME(return_type, object_type, task_name) __##object_type##_##task_name##__

#define ASYNC_PROTOTYPE(result_type, object_type, task_name, ...) async_return_t __##object_type##_##task_name##__(result_type *_result, struct async_task *parent, object_type *self, ##__VA_ARGS__)
#define ASYNC_PTR(result_type, object_type, task_name, ...) async_return_t (*__##object_type##_##task_name##__)(result_type *_result, struct async_task *parent, object_type *self, ##__VA_ARGS__)

#define ASYNC_EXIT(ret) {}

#define ASYNC_MEMBER_PROTOTYPE(result_type, task_name, ...) async_task __##task_name##__; async_return_t task_name(result_type *_result, struct async_task *parent, ##__VA_ARGS__)

#define ASYNC_MEMBER(result_type, object_type, task_name, ...) async_return_t object_type::task_name(result_type *_result, struct async_task *parent, ##__VA_ARGS__){ \
	struct async_task *_async = &this->__##task_name##__; 


#define AWAIT_MEMBER_TASK(result_type, task, ...) ({result_type _async_ret; AWAIT(((task(&_async_ret, _async, ##__VA_ARGS__)) == ASYNC_ENDED)); _async_ret;})

struct async_task {
#ifdef __cplusplus
	async_task(){ LC_INIT(this->lc); parent = 0; }
#endif
  lc_t lc; // line to resume on
  struct async_task *parent; // current caller
};

#define ASYNC_INIT(task)  do { (task)->parent = 0; LC_INIT((task)->lc); } while(0)

#define _ASYNC(result_type, object_type, task_name, struct_name, ...) \
	async_return_t __##object_type##_##task_name##__(result_type *_result, \
	struct async_task *parent, object_type *self, ##__VA_ARGS__){ \
	struct async_task *_async = &self->struct_name; 

#define ASYNC(result_type, object_type, task_name, ...) _ASYNC(result_type, object_type, task_name, task_name, ##__VA_ARGS__)
	
#define ASYNC_INVOKE_ONCE(result_ptr, object_type, task_name, parent, ...) __##object_type##_##task_name##__(result_ptr, parent, __VA_ARGS__)

#define AWAIT_TASK(result_type, object_type, task_name, ...) ({result_type _async_ret; AWAIT(((__##object_type##_##task_name##__(&_async_ret, _async, __VA_ARGS__)) == ASYNC_ENDED)); _async_ret;})

#define ASYNC_INIT(task)  do { (task)->parent = 0; LC_INIT((task)->lc); } while(0)

#define ASYNC_BEGIN() { \
	if((_async)->parent != 0 && (_async)->parent != parent) return ASYNC_WAITING; \
	(_async)->parent = parent; \
	async_return_t ASYNC_YIELD_FLAG = 1; \
	(void)ASYNC_YIELD_FLAG; \
	LC_RESUME((_async)->lc)

#define ASYNC_END(ret)  \
	LC_END((_async)->lc); \
	_async->parent = 0; \
	ASYNC_INIT(_async); \
	if((_result) != 0) *(_result) = ret; \
	return ASYNC_ENDED; \
} }

#define AWAIT(condition)	        \
  do { LC_SET((_async)->lc); if(!(condition)) return ASYNC_WAITING; } while(0)


#define ASYNC_YIELD() do { \
	ASYNC_YIELD_FLAG = 0; \
	LC_SET(_async->lc); \
	if(ASYNC_YIELD_FLAG == 0) return ASYNC_WAITING;\
} while(0)


typedef struct async_process async_process_t; 

struct async_queue {
	struct list_head list; 
}; 

typedef struct async_queue async_queue_t; 

#define ASYNC_GLOBAL_QUEUE __async_global_queue
extern struct async_queue ASYNC_GLOBAL_QUEUE; 

struct async_process {
	struct list_head list; 
	ASYNC_PTR(int, async_process_t, proc); 
	struct async_task task; 
	const char *name; 
}; 

void async_process_init(struct async_process *self, ASYNC_PTR(int, async_process_t, func), const char *name); 
void async_queue_process(async_queue_t *queue, struct async_process *self); 
uint8_t async_queue_run(async_queue_t *queue); 

#define __ASYNC_PROCESS(c, func) static _ASYNC(int, async_process_t, func, task)
	
#define _ASYNC_PROCESS(c, func) __ASYNC_PROCESS(c, func)
#define ASYNC_PROCESS(func) _ASYNC_PROCESS(__COUNTER__, func)

#define ASYNC_QUEUE(queue) static struct async_queue queue; static void __attribute__((constructor)) __init_queue_##queue##__(void){\
	INIT_LIST_HEAD(&queue.list); \
}
#define ASYNC_PROCESS_INIT(ptr, func) async_process_init((ptr), ASYNC_NAME(int, async_process_t, func), #func)
#define ASYNC_RUN_QUEUE(queue) async_queue_run(queue)
#define ASYNC_QUEUE_WORK(queue, ctx) async_queue_process(queue, ctx); 
