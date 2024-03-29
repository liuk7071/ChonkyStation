#include <scheduler.hpp>


void Scheduler::tick(u64 cycles) {
	time += cycles;

	for (int i = 0; i < events.size(); i++) {
		if (time >= events.top().time) {
			(*events.top().functionPtr)(events.top().data);
			events.pop();
		}
		else break;
	}
}

void Scheduler::push(void (*functionPtr)(void*), u64 time, void* data, std::string name) {
	Helpers::debugAssert(events.size() < schedulerMaxEntries, "[  FATAL  ] Queued more than %d scheduler events\n", schedulerMaxEntries);

	events.push({ .functionPtr = functionPtr, .data = data, .time = time, .name = name });
}

void Scheduler::deleteAllEventsOfName(std::string name) {
	auto eventList = events.get_container();
	
	for (int i = 0; i < events.size(); i++) {
		for (auto& i : eventList) {
			if (i.name == name) events.remove(i);
		}
	}
}