#include "cpu.h"
#include <iostream>

cpu::cpu() {
	bus.mem.loadBios();
	pc = 0xbfc00000;
	next_instr = pc + 4;
	for (int i = 0x0; i < 32; i++) {
		regs[i] = 0;
	}

	debug = false;
	exe = false;
	log_kernel = false;
	tty = true;

	delay = false;
}

cpu::~cpu() {

}


void cpu::debug_printf(const char* fmt, ...) {
	if(debug) {
		std::va_list args;
		va_start(args, fmt);
		std::vprintf(fmt, args);
		va_end(args);
	}
}


void cpu::exception(exceptions exc) {
	uint32_t handler = 0;
	
	if ((COP0.regs[12] & (1 << 22)) == 0) {
		handler = 0x80000080;
	} else {
		handler = 0xbfc00180;
	}
	

	auto mode = COP0.regs[12] & 0x3f;
	COP0.regs[12] &= 0x3f;
	COP0.regs[12] |= (mode << 2) & 0x3f;

	COP0.regs[13] = uint32_t(exc) << 2;
	COP0.regs[14] = pc;
	pc = handler;
}

uint32_t cpu::fetch(uint32_t addr) {
	return bus.mem.read32(addr);
}


void cpu::do_dma(int channel) {
	debug = true;
	switch (channel) {		// switch on the channels
	case(2): {	// GPU
		auto sync_mode = (bus.mem.channel2_control >> 9) & 0b11;
		bool incrementing = ((bus.mem.channel2_control >> 1) & 1) == 0;
		auto direction = (bus.mem.channel2_control) & 1;
		uint16_t words = (bus.mem.channel2_block_control) & 0xffff;
		uint32_t addr = bus.mem.channel2_base_address & 0x1ffffc;

		switch (sync_mode) {
		case(1): {
			words *= bus.mem.channel2_block_control >> 16;
			debug_printf("[DMA] Start GPU Block Copy\n");
			switch (direction) {
			case(1):
				debug_printf("[DMA] Transfer direction: ram to device\n");
				debug_printf("[DMA] Transfer size: %d words\n", words);
				while (words > 0) {
					uint32_t current_addr = addr & 0x1ffffc;
					uint32_t data = bus.mem.read32(current_addr);
					bus.Gpu.execute_gp0(data);
					if (incrementing) addr += 4; else addr -= 4;
					words--;
				}
				bus.mem.channel2_control ^= 1UL << 24;
				bus.mem.channel2_control ^= 1UL << 28;
				debug = false;
				return;
			default:
				debug_printf("[DMA] Unhandled Direction");
				exit(0);
			}
		}

		case(2):	// Linked List
			debug_printf("[DMA] Start GPU Linked List\n");
			switch (direction) {
			case(1):
				debug_printf("[DMA] Transfer direction: ram to device\n");
				while (1) {
					uint32_t header = bus.mem.read32(addr);
					auto words = header >> 24;
					while (words > 0) {
						addr += 4;
						uint32_t command = bus.mem.read32(addr);
						addr &= 0x1ffffc;
						bus.Gpu.execute_gp0(command);
						words--;
					}
					if ((header & 0x800000) != 0)
						break;
					addr = header & 0x1ffffc;
				}
				bus.mem.channel2_control ^= 1UL << 24;
				bus.mem.channel2_control ^= 1UL << 28;
				debug_printf("[DMA] GPU Linked List transfer complete\n");
				debug = false;
				return;
			default:
				debug_printf("[DMA] Unhandled Direction\n");
				exit(0);
			}
		default:
			debug_printf("[DMA] Unhandled sync mode (GPU)");
			exit(0);
		}
	}

	case(6): {			// OTC
		auto sync_mode = (bus.mem.channel6_control >> 9) & 0b11;
		bool incrementing = ((bus.mem.channel6_control >> 1) & 1) == 0;
		uint16_t words = (bus.mem.channel6_block_control) & 0xffff;
		auto direction = (bus.mem.channel6_control) & 1;
		uint32_t addr = bus.mem.channel6_base_address;

		switch (sync_mode) {	// switch on the sync mode
		case(0): {			// block dma
			debug_printf("[DMA] Start OTC Block Copy\n");
			uint32_t current_addr = addr & 0x1ffffc;
			switch (direction) {
			case(0):
				debug_printf("[DMA] Transfer direction: device to ram\n");
				debug_printf("[DMA] Transfer size: %d words\n", words);
				while (words >= 0) {
					if (words == 0) {
						bus.mem.write32(current_addr, 0xffffff);
						debug_printf("[DMA] OTC Block Copy completed\n");
						bus.mem.channel6_control ^= 1UL << 24;
						bus.mem.channel6_control ^= 1UL << 28;
						debug = false;
						return;
					}
					bus.mem.write32(current_addr, (addr - 4) & 0x1fffff);
					if (incrementing) addr += 4; else addr -= 4;
					words--;
				}

			default:
				debug_printf("[DMA] Unhandled Direction\n");
				exit(0);
			}

			exit(0);
		}
		default:
			debug_printf("[DMA] Unhandled sync mode (OTC)");
			exit(0);
		}
		exit(0);
		break;
	}
	default:
		printf("[DMA] Unhandled DMA channel");
		exit(0);
	}
}


void cpu::check_dma() {
	bool enabled = ((bus.mem.channel2_control >> 24) & 1) == 1;
	bool trigger = ((bus.mem.channel2_control >> 28) & 1) == 1;
	auto sync_mode = (bus.mem.channel2_control >> 9) & 0b11;

	auto triggered = !(sync_mode == 0 && !trigger);
	if (enabled && triggered) { do_dma(2); return; }

	enabled = ((bus.mem.channel6_control >> 24) & 1) == 1;
	trigger = ((bus.mem.channel6_control >> 28) & 1) == 1;
	sync_mode = (bus.mem.channel6_control >> 9) & 0b11;

	triggered = !(sync_mode == 0 && !trigger);
	if (enabled && triggered) { do_dma(6); return; }
}

void cpu::execute(uint32_t instr) {
	regs[0] = 0; // $zero
	
	bus.mem.gpustat = regs[3];

	if (pc == 0xA0 || pc == 0x800000A0 || pc == 0xA00000A0) {
		if (log_kernel) printf("\nkernel call A(0x%x)", regs[9]);
	}
	if (pc == 0xB0 || pc == 0x800000B0 || pc == 0xA00000B0) {
		if (log_kernel) printf("\nkernel call B(0x%x)", regs[9]);
		if (regs[9] == 0x3d)
			if (tty) printf("%c", regs[4]);
	}
	if (pc == 0xC0 || pc == 0x800000C0 || pc == 0xA00000C0) {
		if (log_kernel) printf("\nkernel call C(0x%x)", regs[9]);
	}

	if (pc == 0x80030000 && exe) {
		debug = false;
		bus.mem.debug = false;
		printf("kernel setup done, sideloading executable\n");
		pc = bus.mem.loadExec();
		exe = false;

		memcpy(&regs[28], &bus.mem.file[0x14], sizeof(uint32_t));
		memcpy(&regs[29], &bus.mem.file[0x30], sizeof(uint32_t));

		return;
	}


	uint8_t primary = instr >> 26;
	uint8_t secondary = instr & 0x3f;
	if (delay != 0) {	// branch delay slot
		pc = jump - 4;
		delay = false;
	}

	switch (primary & 0xf0) {
	case(0x00): {
		switch (primary & 0x0f) {
		case(0x00): {
			switch (secondary & 0xf0) {
			case(0x00): {
				switch (secondary & 0x0f) {
				case(0x00): {	// sll
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t shift_imm = (instr >> 6) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint32_t result = regs[rt] << shift_imm;
					regs[rd] = result;
					debug_printf("sll %s, %s, 0x%.8x\n", reg[rd].c_str(), reg[rt].c_str(), shift_imm);
					pc += 4;
					break;
				}
				case(0x02): { // srl
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint8_t shift_imm = (instr >> 6) & 0x1f;
					regs[rd] = regs[rt] >> shift_imm;
					pc += 4;
					debug_printf("srl 0x%.2X, %s, 0x%.8x\n", reg[rd].c_str(), reg[rt].c_str(), shift_imm);
					break;
				}
				case(0x03): { // sra
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint8_t shift_imm = (instr >> 6) & 0x1f;
					regs[rd] = int32_t(regs[rt]) >> shift_imm;
					pc += 4;
					debug_printf("sra %s, %s, 0x%.8x\n", reg[rd].c_str(), reg[rt].c_str(), shift_imm);
					break;
				}
				case(0x04): { // sllv
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					regs[rd] = regs[rt] << (regs[rs] & 0x1f);
					pc += 4;
					debug_printf("sllv %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					break;
				}
				case(0x06): {	// srlv
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					regs[rd] = regs[rt] >> (regs[rs] & 0x1f);
					pc += 4;
					debug_printf("srlv %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					break;
				}
				case(0x07): { // srav
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					regs[rd] = uint32_t(int32_t(regs[rt]) >> (regs[rs] & 0x1f));
					pc += 4;
					debug_printf("srav %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					break;
				}
				case(0x08): {	// jr
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;

					uint32_t addr = regs[rs];
					if (addr & 3) {
						exception(exceptions::BadAddr);
						return;
					}
					jump = regs[rs];
					debug_printf("jr %s\n", reg[rs].c_str());
					pc += 4;
					delay = true;
					break;
				}
				case(0x09): { // jalr
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					uint32_t addr = regs[rs];
					if (addr & 3) {
						exception(exceptions::BadAddr);
						return;
					}
					jump = regs[rs];
					regs[rd] = pc + 8;
					debug_printf("jalr %s\n", reg[rs].c_str());
					pc += 4;
					delay = true;
					break;
				}
				case(0x0C): { // syscall
					debug_printf("syscall\n");
					exception(exceptions::SysCall);
					//debug = true;
					break;
				}
				case(0x0D): {	// break
					debug_printf("break\n");
					exception(exceptions::Break);
					break;
				}
				default: {
					printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
					   break;
				}
				break;
			}
			case(0x10): {
				switch (secondary & 0x0f) {
				case(0x00): { // mfhi
					uint8_t rd = (instr >> 11) & 0x1f;
					regs[rd] = hi;
					debug_printf("mfhi %s\n", reg[rd].c_str());
					pc += 4;
					break;
				}
				case(0x01): { // mthi
					uint8_t rs = (instr >> 21) & 0x1f;
					hi = regs[rs];
					debug_printf("mthi %s\n", reg[rs].c_str());
					pc += 4;
					break;
				}
				case(0x02): { // mflo
					uint8_t rd = (instr >> 11) & 0x1f;
					regs[rd] = lo;
					debug_printf("mflo %s\n", reg[rd].c_str());
					pc += 4;
					break;
				}
				case(0x03): { // mtlo
					uint8_t rs = (instr >> 21) & 0x1f;
					lo = regs[rs];
					debug_printf("mtlo %s\n", reg[rs].c_str());
					pc += 4;
					break;
				}
				case(0x08): {	// mult
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					int64_t x = int64_t(int32_t(regs[rs]));
					int64_t y = int64_t(int32_t(regs[rt]));
					uint64_t result = uint64_t(x * y);

					hi = uint32_t((result >> 32) & 0xffffffff);
					lo = uint32_t(result & 0xffffffff);
					debug_printf("mult %s, %s", reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x09): {	// multu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					uint64_t x = uint64_t(regs[rs]);
					uint64_t y = uint64_t(regs[rt]);
					uint64_t result = x * y;

					hi = uint32_t(result >> 32);
					lo = uint32_t(result);
					debug_printf("multu %s, %s", reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x0A): { // div
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					int32_t n = int32_t(regs[rs]);
					int32_t d = int32_t(regs[rt]);
					debug_printf("div %s, %s\n", reg[rs].c_str(), reg[rt].c_str());

					if (d == 0) {
						hi = uint32_t(n);
						if (n >= 0) {
							lo = 0xffffffff;
						}
						else {
							lo = 1;
						}
						pc += 4;
						break;
					}
					if (uint32_t(n) == 0x80000000 && d == -1) {
						hi = 0;
						lo = 0x80000000;
						pc += 4;
						break;
					}
					hi = uint32_t(n % d);
					lo = uint32_t(n / d);

					pc += 4;
					break;
				}
				case(0x0B): { // divu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					debug_printf("divu %s, %s\n", reg[rs].c_str(), reg[rt].c_str());
					if (regs[rt] == 0) {
						hi = regs[rs];
						lo = 0xffffffff;
						pc += 4;
						break;
					}

					hi = regs[rs] % regs[rt];
					lo = regs[rs] / regs[rt];

					pc += 4;
					break;
				}
				default: {
					printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
					   break;
				}
				break;
			}
			case(0x20): {
				switch (secondary & 0x0f) {
				case(0x00): {	// add
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
					debug_printf("add %s, %s, 0x%.4x", reg[rs].c_str(), reg[rt].c_str(), imm);
					uint32_t result = regs[rs] + regs[rt];
					if (((regs[rs] >> 31) == (regs[rt] >> 31)) && ((regs[rs] >> 31) != (result >> 31))) {
						debug_printf(" add exception");
						exception(exceptions::Overflow);
						break;
					}
					debug_printf("\n");
					regs[rd] = result;
					pc += 4;
					break;
				}
				case(0x02): { // sub
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					uint32_t result = regs[rs] - regs[rt];
					debug_printf("sub %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					if (((regs[rs] ^ result) & (~regs[rt] ^ result)) >> 31) { // overflow
						exception(exceptions::Overflow);
						break;
					}
					regs[rd] = result;
					pc += 4;
					break;
				}

				case(0x03): { // subu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					regs[rd] = regs[rs] - regs[rt];
					debug_printf("subu %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x04): {	// and
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = regs[rs] & regs[rt];
					debug_printf("and %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x05): {	// or
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = regs[rs] | regs[rt];
					debug_printf("or %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x06): {	// xor
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = regs[rs] ^ regs[rt];
					debug_printf("xor %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x07): { // nor
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = ~(regs[rs] | regs[rt]);
					debug_printf("nor %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					pc += 4;
					break;
				}
				case(0x01): { // addu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					regs[rd] = regs[rs] + regs[rt];
					pc += 4;
					debug_printf("addu %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					break;
				}
				case(0x0A): { // slt
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					debug_printf("slt %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					regs[rd] = int32_t(regs[rs]) < int32_t(regs[rt]);
					pc += 4;
					break;
				}
				case(0x0B): {	// sltu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					debug_printf("sltu %s, %s, %s\n", reg[rd].c_str(), reg[rs].c_str(), reg[rt].c_str());
					regs[rd] = regs[rs] < regs[rt];
					pc += 4;
					break;
				}
				default: {
					printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
					   break;
				}
				break;
			}
			case(0x30): {
				switch (secondary & 0x0f) {
				default: {
					printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
					   break;
				}
				break;
			}

			default: {
				printf("Unknown subinstruction: 0x%.2X\n", secondary);
				exit(0);
			}
				   break;
			}

			break;
		}
		case(0x01): {	// BxxZ
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			int32_t signed_rs = int32_t(regs[rs]);
			auto bit16 = (instr >> 16) & 1;
			auto bit20 = (instr >> 20) & 1;
			if (bit16 == 0) {		// bltz
				if (bit20 == 1) regs[0x1f] = pc + 4; // check if link (bltzal)
				debug_printf("BxxZ %s, 0x%.4x", reg[rs].c_str(), sign_extended_imm);
				if (signed_rs < 0) {
					jump = (pc + 4) + (sign_extended_imm << 2);
					delay = true;
					debug_printf(" branched\n");
				}
				else { debug_printf("\n"); }
				pc += 4;
				break;
			}

			if (bit16 == 1) {		// bgez
				debug_printf("BxxZ %s, 0x%.4x", reg[rs].c_str(), sign_extended_imm);
				if (bit20 == 1) regs[0x1f] = pc + 4; // check if link (bgezal)
				if (signed_rs >= 0) {
					jump = (pc + 4) + (sign_extended_imm << 2);
					delay = true;
					debug_printf(" branched\n");
				}
				else { debug_printf("\n"); }
				pc += 4;
				break;
			}
		}
		case(0x02): { // j
			uint32_t jump_imm = instr & 0x3ffffff;
			jump = (pc & 0xf0000000) | (jump_imm << 2);
			debug_printf("j 0x%.8x\n", jump_imm);
			delay = true;
			pc += 4;
			break;
		}
		case(0x03): {	// jal
			uint32_t jump_imm = instr & 0x3ffffff;
			jump = ((pc) & 0xf0000000) | (jump_imm << 2);
			debug_printf("jal 0x%.8x\n", jump_imm);
			regs[0x1f] = pc + 8;
			pc += 4;
			delay = true;
			break;
		}
		case(0x04): {	// beq
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			debug_printf("beq %s, %s, 0x%.4x", reg[rs].c_str(), reg[rt].c_str(), sign_extended_imm);
			if (regs[rs] == regs[rt]) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				delay = true;
				debug_printf(" branched\n");
			}
			else { debug_printf("\n"); }
			pc += 4;
			break;
		}
		case(0x05): {	// bne
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			debug_printf("bne %s, %s, 0x%.4x", reg[rs].c_str(), reg[rt].c_str(), sign_extended_imm);
			if (regs[rs] != regs[rt]) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				delay = true;
				debug_printf(" branched\n");
			}
			else { debug_printf("\n"); }
			pc += 4;
			break;
		}
		case(0x06): {	// blez
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			int32_t signed_rs = int32_t(regs[rs]);
			debug_printf("blez %s, 0x%.4x", reg[rs].c_str(), sign_extended_imm);
			if (signed_rs <= 0) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				delay = true;
				debug_printf(" branched\n");
			}
			else { debug_printf("\n"); }
			pc += 4;
			break;
		}
		case(0x07): {	// bgtz
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			int32_t signed_rs = int32_t(regs[rs]);
			debug_printf("bgtz %s, 0x%.4x", reg[rs].c_str(), sign_extended_imm);
			if (signed_rs > 0) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				delay = true;
				debug_printf(" branched\n");
			}
			else { debug_printf("\n"); }
			pc += 4;
			break;
		}
		case(0x08): { // addi
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			debug_printf("addi %s, %s, 0x%.4x", reg[rs].c_str(), reg[rt].c_str(), imm);
			uint32_t result = regs[rs] + sign_extended_imm;
			if (((regs[rs] >> 31) == (sign_extended_imm >> 31)) && ((regs[rs] >> 31) != (result >> 31))) {
				debug_printf(" addi exception");
				exception(exceptions::Overflow);
				break;
			}
			debug_printf("\n");
			regs[rt] = result;
			pc += 4;
			break;
		}
		case(0x09): { // addiu
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			regs[rt] = regs[rs] + sign_extended_imm;
			debug_printf("addiu %s, %s, 0x%.4x\n", reg[rs].c_str(), reg[rt].c_str(), imm);
			pc += 4;
			break;
		}
		case(0x0A): {	// slti
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			int32_t signed_sign_extended_imm = int32_t(uint32_t(int32_t(int16_t(imm))));

			int32_t signed_rs = int32_t(regs[rs]);
			regs[rt] = 0;
			if (signed_rs < signed_sign_extended_imm)
				regs[rt] = 1;
			pc += 4;
			debug_printf("slti %s, %s, 0x%.4X\n", reg[rs].c_str(), reg[rt].c_str(), imm);
			break;
		}
		case(0x0B): { // sltiu
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));

			regs[rt] = regs[rs] < sign_extended_imm;
			//regs[rt] = 0;
			//if (regs[rs] < sign_extended_imm)
			//	regs[rt] = 1;
			pc += 4;
			debug_printf("sltiu %s, %s, 0x%.4X\n", reg[rs].c_str(), reg[rt].c_str(), imm);
			break;
		}
		case(0x0C): {	// andi
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			debug_printf("andi %s, %s, 0x%.4x\n", reg[rs].c_str(), reg[rt].c_str(), imm);
			regs[rt] = (regs[rs] & imm);
			pc += 4;
			break;
		}
		case(0x0D): {	// ori
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			debug_printf("ori %s, %s, 0x%.4x\n", reg[rs].c_str(), reg[rt].c_str(), imm);
			regs[rt] = (regs[rs] | imm);
			pc += 4;
			break;
		}
		case(0x0E): {
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			debug_printf("xori %s, %s, 0x%.4x\n", reg[rs].c_str(), reg[rt].c_str(), imm);
			regs[rt] = (regs[rs] ^ imm);
			pc += 4;
			break;
		}
		case(0x0F): {	// lui
			uint8_t rt = (instr >> 16) & 0x1f;
			uint32_t imm = instr & 0xffff;
			debug_printf("lui %s, 0x%.4x\n", reg[rt].c_str(), imm);
			imm <<= 16;
			regs[rt] = imm;
			pc += 4;
			break;
		}

		default:
			printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		break;
	}
	case(0x10): {
		switch (primary & 0x0f) {
		case(0x00): {	// COP 0
			uint8_t cop_instr = instr >> 21;
			switch (cop_instr) {
			case(0b00000): { // mfc0
				uint8_t rs = (instr >> 21) & 0x1f;
				uint8_t rd = (instr >> 11) & 0x1f;
				uint8_t rt = (instr >> 16) & 0x1f;
				uint16_t imm = instr & 0xffff;
				regs[rt] = COP0.regs[rd];
				debug_printf("mfc0 %s, %s\n", reg[rd].c_str(), reg[rt].c_str());
				break;
			}
			case(0b00100): { // mtc0
				uint8_t rs = (instr >> 21) & 0x1f;
				uint8_t rd = (instr >> 11) & 0x1f;
				uint8_t rt = (instr >> 16) & 0x1f;
				uint16_t imm = instr & 0xffff;
				COP0.regs[rd] = regs[rt];
				debug_printf("mtc0 %s, %s\n", reg[rd].c_str(), reg[rt].c_str());
				break;
			}
			case(0b10000): { // rfe
				if ((instr & 0x3f) != 0b010000) {
					printf("Invalid RFE");
					exit(0);
				}
				debug_printf("rfe");
				auto mode = COP0.regs[12] & 0x3f;
				COP0.regs[12] &= ~0x3f;
				COP0.regs[12] |= mode >> 2;
				break;
			}
			default:
				printf("Unknown coprocessor instruction: 0x%.2x", cop_instr);
				exit(0);
			}
			pc += 4;
			break;
		}
		case(0x01): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x03): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x04): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x05): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x06): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x07): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x08): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x09): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0A): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0C): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0D): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0E): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0f): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		default:
			printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		break;
	}

	case(0x20): {
		switch (primary & 0x0f) {
		case(0x00): {	// lb
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));

			uint32_t addr = regs[rs] + sign_extended_imm;

			if ((COP0.regs[12] & 0x10000) == 1) {
				debug_printf(" cache isolated, ignoring load\n");
				pc += 4;
				return;
			}

			uint8_t byte = bus.mem.read(addr);
			regs[rt] = uint32_t(int32_t(int8_t(byte)));

			debug_printf("lb %s, 0x%.4x(%s)\n", reg[rt].c_str(), imm, reg[rs].c_str());
			pc += 4;
			break;
		}
		case(0x01): {	// lh
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("lh %s, 0x%.4x(%s)\n", reg[rt].c_str(), imm, reg[rs].c_str());
			if ((COP0.regs[12] & 0x10000) == 0) {
				int16_t data = int16_t(bus.mem.read16(addr));
				regs[rt] = uint32_t(int32_t(data));
				debug_printf("\n");
			}
			else {
				debug_printf(" cache isolated, ignoring load\n");
			}
			pc += 4;
			break;
		}
		case(0x02): {	// lwl
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));

			uint32_t addr = regs[rs] + sign_extended_imm;
			uint32_t aligned_addr = addr & ~3;
			uint32_t aligned_word = bus.mem.read32(aligned_addr);
			debug_printf("lwl %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			pc += 4;
			break;
			switch (addr & 3) {
			case 0:
				regs[rt] = aligned_word << 24;
				break;
			case 1:
				regs[rt] = aligned_word << 16;
				break;
			case 2:
				regs[rt] = aligned_word << 8;
				break;
			case 3:
				regs[rt] = aligned_word << 0;
				break;
			}

			lwl = true;
			pc += 4;
			break;
		}
		case(0x03): {	// lw  
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("lw %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			if ((COP0.regs[12] & 0x10000) == 0) {
				regs[rt] = bus.mem.read32(addr);
				debug_printf("\n");
			}
			else {
				debug_printf(" cache isolated, ignoring load\n");
			}
			pc += 4;
			break;
		}
		case(0x04): {	// lbu
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("lbu %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			if ((COP0.regs[12] & 0x10000) == 0) {
				uint8_t byte = bus.mem.read(addr);
				regs[rt] = uint32_t(byte);
				debug_printf("\n");
			}
			else {
				debug_printf(" cache isolated, ignoring load\n");
			}
			pc += 4;
			break;
		}
		case(0x05): {	// lhu
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("lhu %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			if ((COP0.regs[12] & 0x10000) == 0) {
				uint16_t data = bus.mem.read16(addr);
				regs[rt] = uint32_t(data);
				debug_printf("\n");
			}
			else {
				debug_printf(" cache isolated, ignoring load\n");
			}
			pc += 4;
			break;
		}
		case(0x06): {	// lwr
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));

			uint32_t addr = regs[rs] + sign_extended_imm;
			uint32_t aligned_addr = addr & ~3;
			uint32_t aligned_word = bus.mem.read32(aligned_addr);
			debug_printf("lwr %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			pc += 4;
			break;
			if (lwl) {
				switch (addr & 3) {
				case 0:
					regs[rt] = (regs[rt] & 0x00000000) | (aligned_word >> 0);
					break;
				case 1:
					regs[rt] = (regs[rt] & 0xff000000) | (aligned_word >> 8);
					break;
				case 2:
					regs[rt] = (regs[rt] & 0xffff0000) | (aligned_word >> 16);
					break;
				case 3:
					regs[rt] = (regs[rt] & 0xffffff00) | (aligned_word >> 24);
					break;
				}
			}
			else {
				switch (addr & 3) {
				case 0:
					regs[rt] = aligned_word >> 0;
					break;
				case 1:
					regs[rt] = aligned_word >> 8;
					break;
				case 2:
					regs[rt] = aligned_word >> 16;
					break;
				case 3:
					regs[rt] = aligned_word >> 24;
					break;
				}
			}

			lwr = true;
			pc += 4;
			break;
		}
		case(0x07): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x08): {	// sb
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("sb %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			if ((COP0.regs[12] & 0x10000) == 0) {
				bus.mem.write(addr, uint8_t(regs[rt]), true);
				debug_printf("\n");
			}
			else {
				debug_printf(" cache isolated, ignoring write\n");
			}
			pc += 4;
			break;
		}
		case(0x09): { // sh
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("sh %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			if ((COP0.regs[12] & 0x10000) == 0) {
				bus.mem.write16(addr, uint16_t(regs[rt]));
				debug_printf("\n");
			}
			else {
				debug_printf(" cache isolated, ignoring write\n");
			}
			pc += 4;
			break;

		}
		case(0x0A): {	// swl
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));

			uint32_t addr = regs[rs] + sign_extended_imm;
			uint32_t aligned_addr = addr & ~3;

			uint32_t mem = bus.mem.read32(aligned_addr);
			uint32_t val = 0;

			switch (addr & 3) {
			case 0:
				val = (mem & 0xffffff00) | (regs[rt] >> 24);
				break;
			case 1:
				val = (mem & 0xffff0000) | (regs[rt] >> 16);
				break;
			case 2:
				val = (mem & 0xff000000) | (regs[rt] >> 8);
				break;
			case 3:
				val = (mem & 0x00000000) | (regs[rt] >> 0);
				break;
			}

			bus.mem.write32(addr, val);
			debug_printf("swl %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			pc += 4;
			break;
		}
		case(0x0B): { // sw
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			debug_printf("sw %s, 0x%.4x(%s)\n", reg[rt].c_str(), imm, reg[rs].c_str());

			if (addr == 0x1f801810) {	// handle gp0 command
				bus.Gpu.execute_gp0(regs[rt]);
				pc += 4;
				break;
			}

			if (addr == 0x1f801814) {	// handle gp1 command
				bus.Gpu.execute_gp1(regs[rt]);
				pc += 4;
				break;
			}

			if ((COP0.regs[12] & 0x10000) == 0) {
				bus.mem.write32(addr, regs[rt]);
				debug_printf("\n");
			}
			
			
			
			pc += 4;
			break;
		}
		case(0x0C): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0D): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0E): {	// swr
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));

			uint32_t addr = regs[rs] + sign_extended_imm;
			uint32_t aligned_addr = addr & ~3;

			uint32_t mem = bus.mem.read32(aligned_addr);
			uint32_t val = 0;

			switch (addr & 3) {
			case 0:
				val = (mem & 0x00000000) | (regs[rt] << 0);
				break;
			case 1:
				val = (mem & 0x000000ff) | (regs[rt] >> 8);
				break;
			case 2:
				val = (mem & 0x0000ffff) | (regs[rt] >> 16);
				break;
			case 3:
				val = (mem & 0x00ffffff) | (regs[rt] >> 24);
				break;
			}

			bus.mem.write32(addr, val);
			debug_printf("swr %s, 0x%.4x(%s)", reg[rt].c_str(), imm, reg[rs].c_str());
			pc += 4;
			break;
		}
		case(0x0f): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		default:
			printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		break;
	}

	case(0x30): {
		switch (primary & 0x0f) {
		case(0x00): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x01): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x03): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x04): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x05): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x06): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x07): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x08): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x09): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0A): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0C): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0D): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0E): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0f): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		default:
			printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
	}




	default: {
		printf("Unknown instruction: 0x%.2X\n", primary);
		exit(0);
	}
	}

	if (lwl) lwl = false;
	if (lwr) lwr = false;
}