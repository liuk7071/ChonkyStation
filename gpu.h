#pragma once
#include <stdint.h>
#include "memory.h"
class gpu
{
public:
	gpu();
	void execute(uint32_t command);
	void connectMem(memory* memory);
public:
	memory mem;
	
};

