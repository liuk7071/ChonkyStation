#pragma once
#include <stdio.h>
#include <string>

#define MAX_ENTRIES 16

class scheduler
{
public:
	scheduler();

	struct event {
		void (*function_ptr)(void*);
		void* data;
		uint64_t time;
	};
	event events[MAX_ENTRIES];
	int scheduled = 0; // Counter of how many events are scheduled

	void push(void (*ptr)(void*), int time, void* classptr);

	uint64_t time = 0; // Current time
	void tick(uint64_t cycles);
};

