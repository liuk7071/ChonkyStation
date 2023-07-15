#include "interpreter.hpp"


void Interpreter::step(CpuCore* core, Memory* mem) {
	CpuCore::Instruction instr = { .raw = mem->read<u32>(core->pc) };

	core->disassemble(instr);

	auto& gprs = core->gprs;

	switch (instr.primaryOpc) {
	case CpuCore::Opcode::LUI: {
		gprs[instr.rt] = instr.imm << 16;
		break;
	}
	default:
		Helpers::panic("[FATAL] Unimplemented instruction 0x%02x (raw: 0x%08x)\n", instr.primaryOpc.Value(), instr.raw);
	}

	core->pc += 4;
}