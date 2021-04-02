#include "cop0.h"


cop0::cop0() {
	regs[15] = 0x42069420;	// funni number
}
cop0::~cop0() {

}


void cop0::execute(uint32_t instr, uint32_t cpu_regs[]) {
	
	
}