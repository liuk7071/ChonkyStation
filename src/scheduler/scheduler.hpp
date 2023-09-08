#pragma once

#include <helpers.hpp>
#include <queue>


constexpr auto schedulerMaxEntries = 64;

class Scheduler {
public:
	u64 time = 0;
	void tick(u64 cycles);

	struct Event {
		void (*functionPtr)(void*);
		void* data;
		u64 time;

		bool operator>(const Event& other) const {
			return time > other.time;
		}
	};

	std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events;
	void push(void (*functionPtr)(void*), u64 time, void* data);
};