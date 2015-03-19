#pragma once

#include "thread/pt.h"
#include "thread/async.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct libk_thread {
	struct list_head list; 
	char (*proc)(struct libk_thread *kthread, struct pt *self); 
	struct pt thread; 
	// debug
	#ifdef LIBK_DEBUG_THREADS
	long unsigned int time; 
	long unsigned int framecount, timecount; 
	const char *name; 
	#endif
}; 

void libk_create_thread(struct libk_thread *self, char (*proc)(struct libk_thread *kthread, struct pt *self), const char *name); 
/// unlinks the thread from the run queue so that it doesn't get called.
void libk_unlink_thread(struct libk_thread *self); 
/// picks the next thread to run
//void libk_schedule(void); 
//void libk_schedule_thread(struct pt *thread, void (*proc)(void *arg), void *arg); 
/// returns currently running thread, or 0 if we are outside of any thread context
struct pt *libk_current_thread(void); 
/// does a scheduling pass. Should only be called from top level code and never a thread. 
uint8_t libk_schedule(void); 
/// runs main loop. Never returns. 
void libk_run(void); 

// ** UTILITIES **
uint32_t libk_get_fps(void); 
void libk_print_info(void); 
// ===============

#define __LIBK_THREAD(c, func) static struct libk_thread __thread##c##__; \
	static PT_THREAD(_##func##_thread(struct libk_thread __attribute__((unused)) *thr, struct pt *)); \
	__attribute__((constructor)) static void __init__thread##c##__(void){ libk_create_thread(&__thread##c##__, _##func##_thread, #func); } \
	static PT_THREAD(_##func##_thread(struct libk_thread __attribute__((unused)) *kthread, struct pt *pt))

#define _LIBK_THREAD(c, func) __LIBK_THREAD(c, func)

/// macro for creating a thread in code. It creates a static object and automatically registers the
/// thread with the run queue. 
#define LIBK_THREAD(func) _LIBK_THREAD(__COUNTER__, func)

#define PT_SLEEP(pt, var, time_us) { var = timestamp_from_now_us(time_us); PT_WAIT_UNTIL(pt, timestamp_expired(var)); }

#ifdef __cplusplus
}
#endif
