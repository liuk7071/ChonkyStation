#pragma once
#include <stdio.h>
#include <string>
#include <queue>

#define MAX_ENTRIES 64

class scheduler
{
public:
	scheduler();

	struct event {
		void (*function_ptr)(void*);
		void* data;
		uint64_t time;
		std::string name;
		bool operator>(const event &other) const {
			return time > other.time;
		}
	};
	std::priority_queue<event, std::vector<event>, std::greater<event>> events;
	//event events[MAX_ENTRIES];
	int scheduled = 0; // Counter of how many events are scheduled

	void push(void (*ptr)(void*), uint64_t time, void* classptr, const char* name = "Unspecified");

	uint64_t time = 0; // Current time
	void tick(uint64_t cycles);
};

