#include "interpreter.hpp"


void Interpreter::step(CpuCore* core, Memory* mem) {
	u32 instr = mem->read<u32>(core->pc);
	// TODO
}