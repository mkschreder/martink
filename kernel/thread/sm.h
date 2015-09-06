// state machine implementation by Martin Schröder
#pragma once 


typedef void *state_t; 

typedef int sm_result_t; 

struct state_machine {
	state_t state_entry; 
	timestamp_t time; 
}; 

#define SM_INIT(sm) do { (sm)->state_entry = 0; (sm)->resume_point = 0; } while(0)
#define SM(sm, entry) struct state_machine *_current_machine = sm; \
	char _resume = 1; if((sm)->state_entry) goto *((sm)->state_entry); \
	else goto *(&&entry); 
#define SM_STATE(name) _resume = 0; name: if(_resume)
#define SM_ENTER(state) do { _current_machine->state_entry = &&state; return 1; } while(0)

#define _CONCAT2(s1, s2) s1##s2
#define _CONCAT(s1, s2) _CONCAT2(s1, s2)

#define SM_YIELD() do { \
	_current_machine->state_entry = &&_CONCAT(SM_LABEL, __LINE__);	\
	return 1; \
	_CONCAT(SM_LABEL, __LINE__): {}\
} while(0)

#define SM_SLEEP(delay_us) do { for(_current_machine->time = timestamp_from_now_us(delay_us); !timestamp_expired(_current_machine->time);) SM_YIELD(); } while(0)
#define SM_RESULT() (_current_machine->state_entry = 0, (0))
