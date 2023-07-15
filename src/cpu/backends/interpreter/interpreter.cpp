#include "interpreter.hpp"


void Interpreter::step(CpuCore* core, Memory* mem) {
	CpuCore::Instruction instr = { .raw = mem->read<u32>(core->pc) };

	core->disassemble(instr);

	auto& gprs = core->gprs;

	switch (instr.primaryOpc) {
	case CpuCore::Opcode::SPECIAL: {
		switch (instr.secondaryOpc) {
		case CpuCore::SPECIALOpcode::SLL: {
			gprs[instr.rd] = gprs[instr.rt] << instr.shiftImm;
			break;
		}
		case CpuCore::SPECIALOpcode::SRL: {
			gprs[instr.rd] = gprs[instr.rt] >> instr.shiftImm;
			break;
		}
		case CpuCore::SPECIALOpcode::SRA: {
			gprs[instr.rd] = (s32)gprs[instr.rt] >> instr.shiftImm;
			break;
		}
		case CpuCore::SPECIALOpcode::SLLV: {
			gprs[instr.rd] = gprs[instr.rt] << (gprs[instr.rs] & 0x1f);
			break;
		}
		case CpuCore::SPECIALOpcode::SRLV: {
			gprs[instr.rd] = gprs[instr.rt] >> (gprs[instr.rs] & 0x1f);
			break;
		}
		case CpuCore::SPECIALOpcode::SRAV: {
			gprs[instr.rd] = (s32)gprs[instr.rt] >> (gprs[instr.rs] & 0x1f);
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented secondary instruction 0x%02x (raw: 0x%08x)\n", instr.secondaryOpc.Value(), instr.raw);
		}
		break;
	}
	case CpuCore::Opcode::ANDI: {
		gprs[instr.rt] = gprs[instr.rs] & instr.imm;
		break;
	}
	case CpuCore::Opcode::ORI: {
		gprs[instr.rt] = gprs[instr.rs] | instr.imm;
		break;
	}
	case CpuCore::Opcode::XORI: {
		gprs[instr.rt] = gprs[instr.rs] ^ instr.imm;
		break;
	}
	case CpuCore::Opcode::LUI: {
		gprs[instr.rt] = instr.imm << 16;
		break;
	}
	default:
		Helpers::panic("[FATAL] Unimplemented primary instruction 0x%02x (raw: 0x%08x)\n", instr.primaryOpc.Value(), instr.raw);
	}

	core->pc += 4;
}