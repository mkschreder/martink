#pragma once

#include "../thread/async.h"

template<class Parent, class Member>
inline Parent *cpp_container_of(const Member *member, const Member Parent::*   ptr_to_member)
{
   return (Parent*)((const char*)member -
      offset_from_pointer_to_member(ptr_to_member));
}

template<class Parent, class Member>
inline const char* offset_from_pointer_to_member(const Member Parent::* ptr_to_member)
{
   //The implementation of a pointer to member is compiler dependent.
   #if defined(BOOST_INTRUSIVE_MSVC_COMPLIANT_PTR_TO_MEMBER)
   //msvc compliant compilers use their the first 32 bits as offset (even in 64 bit mode)
   return *(const boost::int32_t*)(void*)&ptr_to_member;
   //This works with gcc, msvc, ac++, ibmcpp
   #elif defined(__GNUC__)   || defined(__HP_aCC) || defined(BOOST_INTEL) || \
     defined(__IBMCPP__) || defined(__DECCXX)
   const Parent * const parent = 0;
   const char *const member = reinterpret_cast<const char*>(&(parent->*ptr_to_member));
   return (const char*)(member - reinterpret_cast<const char*>(parent));
   #else
   //This is the traditional C-front approach: __MWERKS__, __DMC__, __SUNPRO_CC
   return (*(const std::ptrdiff_t*)(void*)&ptr_to_member) - 1;
   #endif
}

class AsyncProcess; 

//ASYNC_PROTOTYPE(int, async_process_t, AsyncProcess_task); 

#define ASYNC_CLASS_MAIN_DECLARE() virtual ASYNC_PROTOTYPE(int, async_process_t, ThreadProc)
//ASYNC_MEMBER_PROTOTYPE(int, ThreadProc)
//sync_return_t ThreadProc(int *ret, struct async_task *parent)
#define ASYNC_CLASS_MAIN(objname) ASYNC_MEMBER(int, AsyncProcess, ThreadProc) 
//async_return_t objname::ThreadProc(int *ret, struct async_task *parent)

class AsyncProcess {
public:
	AsyncProcess(){
		ASYNC_INIT(&this->__ThreadProc__); 
		ASYNC_PROCESS_INIT(&this->process, AsyncProcess_task); 
		exit_code = 0; 
		exited = 0; 
	}
	virtual ~AsyncProcess(){
		ASYNC_KILL_PROCESS(&this->process); 
	}
	
	virtual ASYNC_MEMBER_PROTOTYPE(int, ThreadProc) = 0; 
	//ASYNC_MEMBER_PROTOTYPE(int, ThreadProc) = 0; 
	//virtual async_return_t ThreadProc(int *ret, struct async_task *parent) = 0; 
	struct async_process *GetProcessPtr(){ return &this->process; }
	int Exited() { return exited; }
public:
	static ASYNC_PROCESS(AsyncProcess_task){
		class AsyncProcess *thr = cpp_container_of(__self, &AsyncProcess::process);  
		ASYNC_BEGIN(); 
		thr->exited = 0; 
		thr->exit_code = AWAIT_MEMBER_TASK(int, thr->ThreadProc); 
		thr->exited = 1; 
		ASYNC_END(thr->exit_code); 
	}
protected:
	struct async_task __ThreadProc__; 
	struct async_process process; 
	int exit_code; 
	char exited; 
};

static inline void async_queue_process(async_queue_t *queue, AsyncProcess *self){
	async_queue_process(queue, self->GetProcessPtr()); 
}
