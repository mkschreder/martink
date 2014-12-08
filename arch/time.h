#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t timeout_t;

extern void time_init(void);
extern timeout_t time_get_clock(void); 
extern timeout_t time_us_to_clock(timeout_t us);
extern timeout_t time_clock_to_us(timeout_t clock); 

#define time_before(unknown, known) ((timeout_t)(unknown) - (timeout_t)(known) < 0)
#define time_after(a,b) time_before(b, a)
#define timeout_from_now(us) (time_get_clock() + time_us_to_clock(us))
// timeout expired: can handle overflow of timer correctly
#define timeout_expired(timeout) (time_after(time_get_clock(), timeout))
#define time_between(a, b) (a - b)

void time_delay(timeout_t usec); 
timeout_t time_clock_since(timeout_t clock);


#ifdef __cplusplus
}
#endif
