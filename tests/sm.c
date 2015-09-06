#include <stdio.h>

typedef void *state_t; 

typedef int sm_result_t; 

struct state_machine {
	state_t state_entry; 
	int time; 
}; 

#define SM_INIT(sm) do { (sm)->state_entry = 0; (sm)->resume_point = 0; } while(0)
#define SM(sm, entry) struct state_machine *_current_machine = sm; \
	char _resume = 1; if((sm)->state_entry) goto *((sm)->state_entry); \
	else goto *(&&entry); 
#define SM_BODY() if(_current_machine->resume_point) goto *(_current_machine->resume_point); 
#define SM_STATE(name) _resume = 0; name: if(_resume)
#define SM_ENTER(state) do { _current_machine->state_entry = &&state; return 1; } while(0)

#define _CONCAT2(s1, s2) s1##s2
#define _CONCAT(s1, s2) _CONCAT2(s1, s2)

#define SM_YIELD() do { \
	_current_machine->state_entry = &&_CONCAT(SM_LABEL, __LINE__);	\
	return 1; \
	_CONCAT(SM_LABEL, __LINE__): {}\
} while(0)

#define SM_SLEEP(ticks) do { for(_current_machine->time = 0; _current_machine->time < ticks; _current_machine->time++) SM_YIELD(); } while(0)
#define SM_RESULT() (_current_machine->state_entry = 0, (0))

struct myobject {
	struct state_machine sm; 
	int count; 
}; 

void myobject_init(struct myobject *self){
	SM_INIT(&self->sm); 
}

sm_result_t myobject_run(struct myobject *self){
	// when inside the state, we have the chance to declare local
	// temporary variables. These will be reinitialized upon each 
	// reentry. 
	int temp = 0; 
	
	// this line jumps to current state or jumps to the default state
	SM(&self->sm, START){
		SM_STATE(START){
			// this will run if there is no resume point
			printf("Starting the state machine\n"); 
			SM_YIELD(); 
			// this will run after second resume
			printf("first reentry\n"); 
			SM_YIELD(); 
			
			// these definitions will not run by themselves when code gets here
			SM_STATE(SUB_START){
				printf("Entered a substate!\n"); 
				for(self->count = 0; self->count < 10; self->count++){
					printf("Count: %d\n", self->count); 
					if(self->count == 5) SM_ENTER(SUB_SECOND); 
					SM_YIELD(); 
				}
			}
			SM_STATE(ERROR){
				printf("ERROR: entered state without explicit command!\n"); 
			}
			SM_STATE(SUB_SECOND){
				printf("Entered sub second\n"); 
				SM_SLEEP(10); 
			}
			SM_ENTER(END); 
		}
		SM_STATE(END){
			printf("Ending the state machine\n"); 
		}
	}
	// this resets the state machine and returns
	return SM_RESULT(); 
}

int main(void){
	struct myobject obj; 
	myobject_init(&obj); 
	for(int c = 0; c < 2; c++){
		while(myobject_run(&obj)) printf("tick\n"); 
	}
}
