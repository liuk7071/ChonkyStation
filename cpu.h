#pragma once
#include <stdint.h>
#include "Bus.h"
class cpu
{
public:
	cpu();
	~cpu();
public:
	Bus bus = Bus();
public:
	uint32_t fetch(uint32_t addr);
	void execute(uint32_t instr);
public:
	// registers
	uint32_t pc;
	uint32_t sp;
	uint32_t zero;
	uint32_t* regs = new uint32_t[27];

};

