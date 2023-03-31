#include "Bus.h"
Bus::Bus() {
	mem.Spu = &Spu;
	mem.Gpu = &Gpu;
	mem.MDEC = &MDEC;
}

Bus::~Bus() {

}

