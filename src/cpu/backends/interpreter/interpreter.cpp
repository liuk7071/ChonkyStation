#include "interpreter.hpp"


void Interpreter::step(CpuCore* core, Memory* mem, Disassembler* disassembler) {
	// Handle interrupts
	core->checkInterrupt(mem->interrupt);
	//	core->pc -= 4;

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

	switch (instr.primaryOpc) {
	case CpuOpcodes::Opcode::SPECIAL: {
		switch (instr.secondaryOpc) {
		case CpuOpcodes::SPECIALOpcode::SLL: {
			gprs[instr.rd] = gprs[instr.rt] << instr.shiftImm;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SRL: {
			gprs[instr.rd] = gprs[instr.rt] >> instr.shiftImm;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SRA: {
			gprs[instr.rd] = (s32)gprs[instr.rt] >> instr.shiftImm;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SLLV: {
			gprs[instr.rd] = gprs[instr.rt] << (gprs[instr.rs] & 0x1f);
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SRLV: {
			gprs[instr.rd] = gprs[instr.rt] >> (gprs[instr.rs] & 0x1f);
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SRAV: {
			gprs[instr.rd] = (s32)gprs[instr.rt] >> (gprs[instr.rs] & 0x1f);
			break;
		}
		case CpuOpcodes::SPECIALOpcode::JR: {
			const u32 addr = gprs[instr.rs];
			if (addr & 3) {
				Helpers::panic("Bad JR addr\n");
			}
			core->nextPc = addr;
			core->branched = true;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::JALR: {
			const u32 addr = gprs[instr.rs];
			if (addr & 3) {
				Helpers::panic("Bad JALR addr\n");
			}
			gprs[CpuOpcodes::CpuReg::RA] = core->nextPc;
			core->nextPc = addr;
			core->branched = true;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SYSCALL: {
			core->exception(CpuCore::Exception::SysCall, true);
			break;
		}
		case CpuOpcodes::SPECIALOpcode::BREAK: {
			core->exception(CpuCore::Exception::Break, true);
			break;
		}
		case CpuOpcodes::SPECIALOpcode::MFHI: {
			gprs[instr.rd] = core->hi;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::MTHI: {
			core->hi = gprs[instr.rs];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::MFLO: {
			gprs[instr.rd] = core->lo;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::MTLO: {
			core->lo = gprs[instr.rs];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::MULT: {
			s64 x = (s64)(s32)gprs[instr.rs];
			s64 y = (s64)(s32)gprs[instr.rt];
			u64 res = x * y;
			core->hi = res >> 32;
			core->lo = res & 0xffffffff;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::MULTU: {
			u64 x = gprs[instr.rs];
			u64 y = gprs[instr.rt];
			u64 res = x * y;
			core->hi = res >> 32;
			core->lo = res & 0xffffffff;
			break;
		}
		case CpuOpcodes::SPECIALOpcode::DIV: {
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
		case CpuOpcodes::SPECIALOpcode::DIVU: {
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
		case CpuOpcodes::SPECIALOpcode::ADD: {
			gprs[instr.rd] = gprs[instr.rs] + gprs[instr.rt];
			// TODO: overflow exception
			break;
		}
		case CpuOpcodes::SPECIALOpcode::ADDU: {
			gprs[instr.rd] = gprs[instr.rs] + gprs[instr.rt];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SUB: {
			gprs[instr.rd] = gprs[instr.rs] - gprs[instr.rt];
			// TODO: overflow exception
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SUBU: {
			gprs[instr.rd] = gprs[instr.rs] - gprs[instr.rt];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::AND: {
			gprs[instr.rd] = gprs[instr.rs] & gprs[instr.rt];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::OR: {
			gprs[instr.rd] = gprs[instr.rs] | gprs[instr.rt];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::XOR: {
			gprs[instr.rd] = gprs[instr.rs] ^ gprs[instr.rt];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::NOR: {
			gprs[instr.rd] = ~(gprs[instr.rs] | gprs[instr.rt]);
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SLT: {
			gprs[instr.rd] = (s32)gprs[instr.rs] < (s32)gprs[instr.rt];
			break;
		}
		case CpuOpcodes::SPECIALOpcode::SLTU: {
			gprs[instr.rd] = gprs[instr.rs] < gprs[instr.rt];
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented secondary instruction 0x%02x (raw: 0x%08x)\n", instr.secondaryOpc.Value(), instr.raw);
		}
		break;
	}
	case CpuOpcodes::Opcode::REGIMM: {
		switch (instr.regimmOpc) {
		case CpuOpcodes::REGIMMOpcode::BLTZ: {
			if ((s32)gprs[instr.rs] < 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		case CpuOpcodes::REGIMMOpcode::BGEZ: {
			if ((s32)gprs[instr.rs] >= 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		case CpuOpcodes::REGIMMOpcode::BLTZAL: {
			gprs[CpuOpcodes::CpuReg::RA] = core->nextPc;
			if ((s32)gprs[instr.rs] < 0) {
				core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
				core->branched = true;
			}
			break;
		}
		case CpuOpcodes::REGIMMOpcode::BGEZAL: {
			gprs[CpuOpcodes::CpuReg::RA] = core->nextPc;
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
	case CpuOpcodes::Opcode::J: {
		core->nextPc = (core->pc & 0xf0000000) | (instr.jumpImm << 2);
		core->branched = true;
		break;
	}
	case CpuOpcodes::Opcode::JAL: {
		gprs[CpuOpcodes::CpuReg::RA] = core->nextPc;
		core->nextPc = (core->pc & 0xf0000000) | (instr.jumpImm << 2);
		core->branched = true;
		break;
	}
	case CpuOpcodes::Opcode::BEQ: {
		if (gprs[instr.rs] == gprs[instr.rt]) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuOpcodes::Opcode::BNE: {
		if (gprs[instr.rs] != gprs[instr.rt]) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuOpcodes::Opcode::BLEZ: {
		if ((s32)gprs[instr.rs] <= 0) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuOpcodes::Opcode::BGTZ: {
		if ((s32)gprs[instr.rs] > 0) {
			core->nextPc = core->pc + ((u32)(s16)instr.imm << 2);
			core->branched = true;
		}
		break;
	}
	case CpuOpcodes::Opcode::ADDI: {
		gprs[instr.rt] = gprs[instr.rs] + (u32)(s16)instr.imm;
		// TODO: overflow exception
		break;
	}
	case CpuOpcodes::Opcode::ADDIU: {
		gprs[instr.rt] = gprs[instr.rs] + (u32)(s16)instr.imm;
		break;
	}
	case CpuOpcodes::Opcode::SLTI: {
		gprs[instr.rt] = (s32)gprs[instr.rs] < (u32)(s16)instr.imm;
		break;
	}
	case CpuOpcodes::Opcode::SLTIU: {
		gprs[instr.rt] = gprs[instr.rs] < instr.imm;
		break;
	}
	case CpuOpcodes::Opcode::ANDI: {
		gprs[instr.rt] = gprs[instr.rs] & instr.imm;
		break;
	}
	case CpuOpcodes::Opcode::ORI: {
		gprs[instr.rt] = gprs[instr.rs] | instr.imm;
		break;
	}
	case CpuOpcodes::Opcode::XORI: {
		gprs[instr.rt] = gprs[instr.rs] ^ instr.imm;
		break;
	}
	case CpuOpcodes::Opcode::LUI: {
		gprs[instr.rt] = instr.imm << 16;
		break;
	}
	case CpuOpcodes::Opcode::COP0: {
		switch (instr.cop0Opc) {
		case CpuOpcodes::COPOpcode::MF: {
			gprs[instr.rt] = core->cop0.read(instr.rd);
			break;
		}
		case CpuOpcodes::COPOpcode::MT: {
			core->cop0.write(instr.rd, gprs[instr.rt]);
			break;
		}
		case CpuOpcodes::COPOpcode::CO: {
			switch (instr.func) {
			case CpuOpcodes::COP0Opcode::RFE: {
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
	case CpuOpcodes::Opcode::LB: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		gprs[instr.rt] = (u32)(s8)mem->read<u8>(addr);
		break;
	}
	case CpuOpcodes::Opcode::LH: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad lh addr 0x%08x\n", addr);
		}
		gprs[instr.rt] = (u32)(s16)mem->read<u16>(addr);
		break;
	}
	case CpuOpcodes::Opcode::LWL: {
		u32 address = gprs[instr.rs] + (u32)(s16)instr.imm;
		const int shift = ((address & 3) ^ 3) * 8;
		u32 dataTemp = mem->read<u32>(address & ~3);
		u32 rtTemp = gprs[instr.rt] & ~(0xffffffff >> shift);
		dataTemp >>= shift;
		gprs[instr.rt] = dataTemp | rtTemp;
		break;
	}
	case CpuOpcodes::Opcode::LW: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 3) {
			Helpers::panic("Bad lw addr 0x%08x\n", addr);
		}
		gprs[instr.rt] = mem->read<u32>(addr);
		break;
	}
	case CpuOpcodes::Opcode::LBU: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		gprs[instr.rt] = mem->read<u8>(addr);
		break;
	}
	case CpuOpcodes::Opcode::LHU: {
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad lhu addr 0x%08x\n", addr);
		}
		gprs[instr.rt] = mem->read<u16>(addr);
		break;
	}
	case CpuOpcodes::Opcode::LWR: {
		u32 address = gprs[instr.rs] + (u32)(s16)instr.imm;
		const int shift = (address & 3) * 8;
		u32 dataTemp = mem->read<u32>(address & ~3);
		u32 rtTemp = gprs[instr.rt] & ~(0xffffffff << shift);
		dataTemp <<= shift;
		gprs[instr.rt] = dataTemp | rtTemp;
		break;
	}
	case CpuOpcodes::Opcode::SB: {
		if (core->cop0.status.isc) return;
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		mem->write<u8>(addr, gprs[instr.rt]);
		break;
	}
	case CpuOpcodes::Opcode::SH: {
		if (core->cop0.status.isc) return;
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 1) {
			Helpers::panic("Bad sh addr 0x%08x\n", addr);
		}
		mem->write<u16>(addr, gprs[instr.rt]);
		break;
	}
	case CpuOpcodes::Opcode::SWL: {
		u32 address = gprs[instr.rs] + (u32)(s16)instr.imm;
		const int shift = ((address & 3) ^ 3) * 8;
		u32 dataTemp = mem->read<u32>(address & ~3);
		u32 rtTemp = gprs[instr.rt] << shift;
		dataTemp &= ~(0xffffffff << shift);
		mem->write<u32>(address & ~3, dataTemp | rtTemp);
		break;
	}
	case CpuOpcodes::Opcode::SW: {
		if (core->cop0.status.isc) break;
		const u32 addr = gprs[instr.rs] + (u32)(s16)instr.imm;
		if (addr & 3) {
			Helpers::panic("Bad sw addr 0x%08x\n", addr);
		}
		mem->write<u32>(addr, gprs[instr.rt]);
		break;
	}
	case CpuOpcodes::Opcode::SWR: {
		u32 address = gprs[instr.rs] + (u32)(s16)instr.imm;
		const int shift = (address & 3) * 8;
		u32 dataTemp = mem->read<u32>(address & ~3);
		u32 rtTemp = gprs[instr.rt] >> shift;
		dataTemp &= ~(0xffffffff >> shift);
		mem->write<u32>(address & ~3, dataTemp | rtTemp);
		break;
	}
	default:
		Helpers::panic("[FATAL] Unimplemented primary instruction 0x%02x (raw: 0x%08x)\n", instr.primaryOpc.Value(), instr.raw);
	}

	core->isDelaySlot = false;
}