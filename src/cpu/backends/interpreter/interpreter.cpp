#include "interpreter.hpp"


void Interpreter::step(CpuCore* core, Memory* mem, Disassembler* disassembler) {
	CpuCore::Instruction instr = { .raw = mem->read<u32>(core->pc) };

	disassembler->disassemble(instr, core);

	auto& gprs = core->gprs;
	gprs[0] = 0;	// $zero

	if (mem->maskAddress(core->pc) == 0xB0) {
		if (gprs[9] == 0x3d)
			std::putc(gprs[4], stdout);
	}

	core->pc = core->nextPc;
	core->nextPc += 4;

	if (core->branched) {
		core->branched = false;
		core->isDelaySlot = true;
	}

	switch ((CpuCore::Opcode)instr.primaryOpc.Value()) {
	case CpuCore::Opcode::SPECIAL: {
		switch ((CpuCore::SPECIALOpcode)instr.secondaryOpc.Value()) {
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
		case CpuCore::SPECIALOpcode::JR: {
			const u32 addr = gprs[instr.rs];
			if (addr & 3) {
				Helpers::panic("Bad JR addr\n");
			}
			core->nextPc = addr;
			core->branched = true;
			break;
		}
		case CpuCore::SPECIALOpcode::JALR: {
			const u32 addr = gprs[instr.rs];
			if (addr & 3) {
				Helpers::panic("Bad JALR addr\n");
			}
			gprs[CpuCore::CpuReg::RA] = core->nextPc;
			core->nextPc = addr;
			core->branched = true;
			break;
		}
		case CpuCore::SPECIALOpcode::SYSCALL: {
			core->pc -= 4;
			core->exception(CpuCore::Exception::SysCall);
			break;
		}
		case CpuCore::SPECIALOpcode::MFHI: {
			gprs[instr.rd] = core->hi;
			break;
		}
		case CpuCore::SPECIALOpcode::MTHI: {
			core->hi = gprs[instr.rs];
			break;
		}
		case CpuCore::SPECIALOpcode::MFLO: {
			gprs[instr.rd] = core->lo;
			break;
		}
		case CpuCore::SPECIALOpcode::MTLO: {
			core->lo = gprs[instr.rs];
			break;
		}
		case CpuCore::SPECIALOpcode::MULT: {
			s64 x = (s64)(s32)gprs[instr.rs];
			s64 y = (s64)(s32)gprs[instr.rt];
			u64 res = x * y;
			core->hi = res >> 32;
			core->lo = res & 0xffffffff;
			break;
		}
		case CpuCore::SPECIALOpcode::MULTU: {
			u64 x = gprs[instr.rs];
			u64 y = gprs[instr.rt];
			u64 res = x * y;
			core->hi = res >> 32;
			core->lo = res & 0xffffffff;
			break;
		}
		case CpuCore::SPECIALOpcode::DIV: {
			const s32 n = (s32)gprs[instr.rs];
			const s32 d = (s32)gprs[instr.rt];

			if (d == 0) {
				core->hi = n;
				if (n >= 0)
					core->lo = -1;
				else
					core->lo = 1;
				break;
			}
			if (n == 0x80000000 && d == -1) {
				core->hi = 0;
				core->lo = 0x80000000;
				break;
			}
			core->hi = n % d;
			core->lo = n / d;
			break;
		}
		case CpuCore::SPECIALOpcode::DIVU: {
			const u32 n = gprs[instr.rs];
			const u32 d = gprs[instr.rt];
			
			if (d == 0) {
				core->hi = n;
				core->lo = 0xffffffff;
				break;
			}
			core->hi = n % d;
			core->lo = n / d;
			break;
		}
		case CpuCore::SPECIALOpcode::ADD: {
			gprs[instr.rd] = gprs[instr.rs] + gprs[instr.rt];
			// TODO: overflow exception
			break;
		}
		case CpuCore::SPECIALOpcode::ADDU: {
			gprs[instr.rd] = gprs[instr.rs] + gprs[instr.rt];
			break;
		}
		case CpuCore::SPECIALOpcode::SUB: {
			gprs[instr.rd] = gprs[instr.rs] - gprs[instr.rt];
			// TODO: overflow exception
			break;
		}
		case CpuCore::SPECIALOpcode::SUBU: {
			gprs[instr.rd] = gprs[instr.rs] - gprs[instr.rt];
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
		case CpuCore::SPECIALOpcode::SLT: {
			gprs[instr.rd] = (s32)gprs[instr.rs] < (s32)gprs[instr.rt];
			break;
		}
		case CpuCore::SPECIALOpcode::SLTU: {
			gprs[instr.rd] = gprs[instr.rs] < gprs[instr.rt];
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented secondary instruction 0x%02x (raw: 0x%08x)\n", instr.secondaryOpc.Value(), instr.raw);
		}
		break;
	}
	case CpuCore::Opcode::REGIMM: {
		switch ((CpuCore::REGIMMOpcode)instr.regimmOpc.Value()) {
		case CpuCore::REGIMMOpcode::BLTZ: {
			if ((s32)gprs[instr.rs] < 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		case CpuCore::REGIMMOpcode::BGEZ: {
			if ((s32)gprs[instr.rs] >= 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		case CpuCore::REGIMMOpcode::BLTZAL: {
			gprs[CpuCore::CpuReg::RA] = core->nextPc;
			if ((s32)gprs[instr.rs] < 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		case CpuCore::REGIMMOpcode::BGEZAL: {
			gprs[CpuCore::CpuReg::RA] = core->nextPc;
			if ((s32)gprs[instr.rs] >= 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		default:
			Helpers::panic("[FATAL] Invalid REGIMM instruction 0x%02x (raw: 0x%08x)\n", instr.regimmOpc.Value(), instr.raw);
		}
		break;
	}
	case CpuCore::Opcode::J: {
		core->nextPc = (core->pc & 0xf0000000) | (instr.jumpImm << 2);
		core->branched = true;
		break;
	}
	case CpuCore::Opcode::JAL: {
		gprs[CpuCore::CpuReg::RA] = core->nextPc;
		core->nextPc = (core->pc & 0xf0000000) | (instr.jumpImm << 2);
		core->branched = true;
		break;
	}
	case CpuCore::Opcode::BEQ: {
		if (gprs[instr.rs] == gprs[instr.rt]) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuCore::Opcode::BNE: {
		if (gprs[instr.rs] != gprs[instr.rt]) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuCore::Opcode::BLEZ: {
		if ((s32)gprs[instr.rs] <= 0) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuCore::Opcode::BGTZ: {
		if ((s32)gprs[instr.rs] > 0) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
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
	case CpuCore::Opcode::SLTI: {
		gprs[instr.rt] = (s32)gprs[instr.rs] < (u32)(s16)instr.imm;
		break;
	}
	case CpuCore::Opcode::SLTIU: {
		gprs[instr.rt] = gprs[instr.rs] < instr.imm;
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
		switch ((CpuCore::COPOpcode)instr.cop0Opc.Value()) {
		case CpuCore::COPOpcode::MF: {
			gprs[instr.rt] = core->cop0.read(instr.rd);
			break;
		}
		case CpuCore::COPOpcode::MT: {
			core->cop0.write(instr.rd, gprs[instr.rt]);
			break;
		}
		case CpuCore::COPOpcode::CO: {
			switch ((CpuCore::COP0Opcode)instr.func.Value()) {
			case CpuCore::COP0Opcode::RFE: {
				core->cop0.status.raw = (core->cop0.status.raw & 0xfffffff0) | ((core->cop0.status.raw & 0x3c) >> 2);
				break;
			}
			default:
				Helpers::panic("[FATAL] Invalid cop0 instruction 0x%02 (raw:0x%08x)\n", instr.func.Value(), instr.raw);
			}
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented cop instruction 0x%02x (raw: 0x%08x)\n", instr.cop0Opc.Value(), instr.raw);
		}
		break;
	}
	case CpuCore::Opcode::LB: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		gprs[instr.rt] = (u32)(s8)mem->read<u8>(addr);
		break;
	}
	case CpuCore::Opcode::LH: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad lh addr 0x%08x\n", addr);
		}
		gprs[instr.rt] = (u32)(s16)mem->read<u16>(addr);
		break;
	}
	case CpuCore::Opcode::LW: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 3) {
			Helpers::panic("Bad lw addr 0x%08x\n", addr);
		}
		gprs[instr.rt] = mem->read<u32>(addr);
		break;
	}
	case CpuCore::Opcode::LBU: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		gprs[instr.rt] = mem->read<u8>(addr);
		break;
	}
	case CpuCore::Opcode::LHU: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad lhu addr 0x%08x\n", addr);
		}
		gprs[instr.rt] = mem->read<u16>(addr);
		break;
	}
	case CpuCore::Opcode::SB: {
		if (core->cop0.status.isc) return;
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		mem->write<u8>(addr, gprs[instr.rt]);
		break;
	}
	case CpuCore::Opcode::SH: {
		if (core->cop0.status.isc) return;
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad sh addr 0x%08x\n", addr);
		}
		mem->write<u16>(addr, gprs[instr.rt]);
		break;
	}
	case CpuCore::Opcode::SW: {
		if (core->cop0.status.isc) break;
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 3) {
			Helpers::panic("Bad sw addr 0x%08x\n", addr);
		}
		mem->write<u32>(addr, gprs[instr.rt]);
		break;
	}
	default:
		Helpers::panic("[FATAL] Unimplemented primary instruction 0x%02x (raw: 0x%08x)\n", instr.primaryOpc.Value(), instr.raw);
	}

	if (core->isDelaySlot) core->isDelaySlot = false;
}