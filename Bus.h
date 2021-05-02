#pragma once
#include <stdint.h>
#include <array>
#include "memory.h"
#include "gpu.h"
#include "cdrom.h"
class Bus
{
public:
	Bus();
	~Bus();
public:
	gpu Gpu = gpu();
	memory mem = memory();
	
public:

};

