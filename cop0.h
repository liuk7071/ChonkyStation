#pragma once
#include <stdint.h>
#include <iostream>
#include "Bus.h"



class cop0
{
public:
	cop0();
	~cop0();
public:
	void execute(uint32_t instr, uint32_t regs[]);

public:
	uint32_t regs[32];
};

