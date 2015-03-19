#pragma once

#include "lc.h"

#define ASYNC_WAITING 0
#define ASYNC_ENDED   3

typedef char async_return_t; 

#ifdef __cplusplus

struct async_task {
  lc_t lc; // line to resume on
  struct async_task *parent; 
	async_task(){ parent = 0; lc = 0; }
};

#define ASYNC_DECL(task_name, ...) async_task __##task_name##__; async_return_t task_name(struct async_task *parent, ##__VA_ARGS__)

#define ASYNC(object_type, task_name, ...) async_return_t object_type::task_name(struct async_task *parent, ##__VA_ARGS__){ \
	struct async_task *_async = &this->__##task_name##__; 

#define AWAIT_TASK(task, ...) AWAIT(((task(_async, __VA_ARGS__)) == ASYNC_ENDED))

//#define ASYNC_INVOKE_ONCE(object_type, task_name, parent, ...) __##object_type##_##task_name##__(parent, __VA_ARGS__)

#else
/*****************
** C implementation
******************/

struct async_task {
  lc_t lc; // line to resume on
  struct async_task *parent; 
};

#define ASYNC_INIT(task)  do { (task)->parent = 0; LC_INIT((task)->lc); } while(0)

#define ASYNC_DECL(task, ...) async_return_t __##object_type##_##task_name##__(struct async_task *parent, object_type *self, ##__VA_ARGS__)

#define ASYNC(object_type, task_name, ...) async_return_t __##object_type##_##task_name##__(struct async_task *parent, object_type *self, ##__VA_ARGS__){ \
	struct async_task *_async = &self->task_name; \

#define ASYNC_INVOKE_ONCE(object_type, task_name, parent, ...) __##object_type##_##task_name##__(parent, __VA_ARGS__)
#define AWAIT_TASK(object_type, task_name, ...) AWAIT(((__##object_type##_##task_name##__(_async, __VA_ARGS__)) == ASYNC_ENDED))

#endif
/************
***************
****************/

#define ASYNC_INIT(task)  do { (task)->parent = 0; LC_INIT((task)->lc); } while(0)

#define ASYNC_BEGIN() { \
	if((_async)->parent != 0 && (_async)->parent != parent) return ASYNC_WAITING; \
	(_async)->parent = parent; \
	async_return_t ASYNC_YIELD_FLAG = 1; \
	(void)ASYNC_YIELD_FLAG; \
	LC_RESUME((_async)->lc)

#define ASYNC_END()  \
	LC_END((_async)->lc); \
	_async->parent = 0; \
	ASYNC_INIT(_async); \
	return ASYNC_ENDED; \
} }

#define AWAIT(condition)	        \
  do { LC_SET((_async)->lc); if(!(condition)) return ASYNC_WAITING; } while(0)


#define ASYNC_YIELD() do { \
	ASYNC_YIELD_FLAG = 0; \
	LC_SET(_async->lc); \
	if(ASYNC_YIELD_FLAG == 0) return ASYNC_WAITING;\
} while(0)
