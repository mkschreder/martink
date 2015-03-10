#include <arch/soc.h>
#include "thread.h"

LIST_HEAD(_running); 
LIST_HEAD(_idle); 
static uint32_t _fps = 0; 
static uint32_t _bfps = 0; 
static struct pt *_current_thread = 0; 

void libk_create_thread(struct libk_thread *self, char (*func)(struct libk_thread *kthr, struct pt *), const char *name){
	PT_INIT(&self->thread); 
	self->proc = func; 
	self->name = name; 
	self->time = 0; 
	list_add_tail(&self->list, &_running); 
}

void libk_unlink_thread(struct libk_thread *self){
	PT_INIT(&self->thread); 
	self->proc = 0; 
	self->name = 0; 
	self->time = 0; 
	list_del_init(&self->list); 
}
/*
void libk_schedule_thread(struct pt *thread, void (*proc)(void *arg), void *arg){
	struct pt *pt = _current_thread; 
	_current_thread = thread; 
	proc(arg); 
	_current_thread = pt; 
}
*/
struct pt *libk_current_thread(void){
	return _current_thread; 
}

static void libk_schedule(void){
	struct list_head *ptr, *n; 
	static timestamp_t timeout = 0; 
	list_for_each_safe(ptr, n, &_running){
		struct libk_thread *thr = container_of(ptr, struct libk_thread, list); 
		timestamp_t time = timestamp_now(); 
		struct pt *_backup = _current_thread; 
		_current_thread = &thr->thread; 
		thr->proc(thr, &thr->thread); 
		_current_thread = _backup; 
		timestamp_t tnow = timestamp_now(); 
		if(tnow > time)
			thr->timecount += tnow - time;
		thr->framecount++; 
	}
	if(timestamp_expired(timeout)){
		list_for_each_safe(ptr, n, &_running){
			struct libk_thread *thr = container_of(ptr, struct libk_thread, list); 
			thr->time = (thr->time + timestamp_ticks_to_us(thr->timecount)) / 2; 
			thr->framecount = 0; 
			thr->timecount = 0; 
		}
		timeout = timestamp_from_now_us(1000000); 
	}
}

void libk_run(void){
	while(1){
		libk_schedule(); 
	}
}

uint32_t libk_get_fps(void){
	return _fps; 
}

void libk_print_info(void){
	struct list_head *ptr, *n; 
	printf("Running threads: \n"); 
	uint16_t count = 0; 
	long unsigned int ustotal = 0; 
	list_for_each_safe(ptr, n, &_running){
		struct libk_thread *thr = container_of(ptr, struct libk_thread, list); 
		long unsigned int time = thr->time; 
		printf("%-16s: %4lu0000us\n", thr->name, time / 10000); 
		count++; 
		ustotal += thr->time; 
	}
	printf("TOTAL: %d threads, %lu us\n", count, ustotal); 
	printf("FPS: %lu\n", libk_get_fps()); 
}

LIBK_THREAD(fps_count){
	static timestamp_t time = 0; 
	_bfps++; 
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
		_fps = _bfps; 
		_bfps = 0; 
		time = timestamp_from_now_us(1000000); 
		PT_YIELD(pt); 
	}
	PT_END(pt); 
}
