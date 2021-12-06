#include "scheduler.h"

scheduler::scheduler() {
	for (int i = 0; i < MAX_ENTRIES; i++) {
		events[i].function_ptr = nullptr;
		events[i].time = 0;
	}
}
void scheduler::push(void (*ptr)(), int time) {
	event Event;
	Event.function_ptr = ptr;
	Event.time = time;
	
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (events[i].time > Event.time) {
			std::memmove(&events[i + 1], &events[i], (MAX_ENTRIES - i - 1) * sizeof(event));
			events[i] = Event;
			scheduled++; return;
		}
	}
	if(scheduled < 12) events[scheduled++] = Event;
}

void scheduler::tick(uint64_t cycles) {
	time += cycles;

	int executed = 0;
	for (int i = 0; i < scheduled; i++) {
		if (time >= events[i].time) {
			(*events[i].function_ptr)();
			executed++;
		}
	}
	std::memmove(&events[0], &events[executed], (scheduled - executed) * sizeof(event));
	scheduled -= executed;
}