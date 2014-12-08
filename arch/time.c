#include <inttypes.h>
#include "time.h"


void time_delay(timeout_t usec){
	timeout_t t = timeout_from_now(usec); 
	while(!timeout_expired(t)); 
}

timeout_t time_clock_since(timeout_t clock){
	return time_get_clock() - clock; 
}
