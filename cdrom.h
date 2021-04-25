#pragma once
#include "memory.h"
class cdrom
{
public:
	cdrom();
public:
	memory* mem;
	void connectMem(memory *_mem);
};

