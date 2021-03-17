#pragma once
#include <stdint.h>
#include "memory.h"
class gpu
{
public:
	gpu();
	void execute_gp0(uint32_t command);
	void execute_gp1(uint32_t command);
	void connectMem(memory* memory);
public:
	memory mem;
	
};

