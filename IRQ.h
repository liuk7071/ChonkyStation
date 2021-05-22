#pragma once
#include "cpu.h"
class IRQ
{
public:
	IRQ();
	void connectCPU(cpu& cpu);
public:
	cpu* CPU;
};

