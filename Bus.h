#pragma once
#include <stdint.h>
#include <array>
#include "memory.h"
#include "gpu.h"
#include "mdec.h"
#include "spu.h"

class Bus
{
public:
	Bus();
	~Bus();
public:
	gpu Gpu = gpu();
	spu Spu = spu();
	memory mem = memory();
	mdec MDEC;
};

