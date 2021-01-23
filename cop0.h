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
	void execute(uint32_t instr, uint32_t* regs);
public:
	uint32_t bpc;
	uint32_t bda;
	uint32_t i;
	uint32_t dcic;
	uint32_t bdam;
	uint32_t bpcm;
	uint32_t sr;
	uint32_t cause;
};

