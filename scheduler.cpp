#include "scheduler.h"

scheduler::scheduler() {

}
void scheduler::push(void (*ptr)(void*), uint64_t time, void* classptr, const char* name) {
	event Event;
	Event.function_ptr = ptr;
	Event.time = time;
	Event.data = classptr;
	Event.name = name;

	//printf("Scheduling event \"%s\" at time %d...\n", Event.name.c_str(), Event.time);
	if (scheduled < MAX_ENTRIES) { 
		events.push(Event);
		scheduled++;
	}
	else {
		printf("Attempted to schedule more than MAX_ENTRIES (%d) events.\n", MAX_ENTRIES);
	}
}

void scheduler::tick(uint64_t cycles) {
	time += cycles;
	int executed = 0;
	for (int i = 0; i < scheduled; i++) {
		if (time >= events.top().time) {
			(*events.top().function_ptr)(events.top().data);
			events.pop();
			executed++;
		}
		else break;
	}
	scheduled -= executed;
}