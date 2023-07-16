#include "interpreter.hpp"


void Interpreter::step(CpuCore* core, Memory* mem) {
	CpuCore::Instruction instr = { .raw = mem->read<u32>(core->pc) };

	core->disassemble(instr);

	core->pc = core->nextPc;

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
		case CpuCore::SPECIALOpcode::AND: {
			gprs[instr.rd] = gprs[instr.rs] & gprs[instr.rt];
			break;
		}
		case CpuCore::SPECIALOpcode::OR: {
			gprs[instr.rd] = gprs[instr.rs] | gprs[instr.rt];
			break;
		}
		case CpuCore::SPECIALOpcode::XOR: {
			gprs[instr.rd] = gprs[instr.rs] ^ gprs[instr.rt];
			break;
		}
		case CpuCore::SPECIALOpcode::NOR: {
			gprs[instr.rd] = ~(gprs[instr.rs] | gprs[instr.rt]);
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented secondary instruction 0x%02x (raw: 0x%08x)\n", instr.secondaryOpc.Value(), instr.raw);
		}
		break;
	}
	case CpuCore::Opcode::J: {
		core->nextPc = (core->pc & 0xf0000000) | (instr.jumpImm << 2);
		break;
	}
	case CpuCore::Opcode::ADDI: {
		gprs[instr.rt] = gprs[instr.rs] + (u32)(s16)instr.imm;
		// TODO: overflow exception
		break;
	}
	case CpuCore::Opcode::ADDIU: {
		gprs[instr.rt] = gprs[instr.rs] + (u32)(s16)instr.imm;
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
	case CpuCore::Opcode::COP0: {
		switch (instr.cop0Opc) {
		case CpuCore::COPOpcode::MT: {
			core->cop0.write(instr.rd, gprs[instr.rt]);
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented cop0 instruction 0x%02x (raw: 0x%08x)\n", instr.cop0Opc.Value(), instr.raw);
		}
		break;
	}
	case CpuCore::Opcode::SB: {
		if (core->cop0.status.isc) return;
		u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		mem->write<u8>(addr, gprs[instr.rt]);
		break;
	}
	case CpuCore::Opcode::SH: {
		if (core->cop0.status.isc) return;
		u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad sh addr 0x%08x\n", addr);
		}
		mem->write<u16>(addr, gprs[instr.rt]);
		break;
	}
	case CpuCore::Opcode::SW: {
		if (core->cop0.status.isc) break;
		u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 3) {
			Helpers::panic("Bad sw addr 0x%08x\n", addr);
		}
		mem->write<u32>(addr, gprs[instr.rt]);
		break;
	}
	default:
		Helpers::panic("[FATAL] Unimplemented primary instruction 0x%02x (raw: 0x%08x)\n", instr.primaryOpc.Value(), instr.raw);
	}

	core->nextPc += 4;
}