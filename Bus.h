#pragma once
#include <stdint.h>
#include <array>
#include "memory.h"
#include "gpu.h"
#include "mdec.h"

class Bus
{
public:
	Bus();
	~Bus();
public:
	gpu Gpu = gpu();
	memory mem = memory();
	mdec MDEC;
};

