#include "Bus.h"
Bus::Bus() {
	mem.Gpu = &Gpu;
	mem.MDEC = &MDEC;
}

Bus::~Bus() {

}

