#include "cpu.h"
#include <iostream>
cpu::cpu() {
	bus.mem.loadBios();
	pc = 0xbfc00000;
	for (int i = 0x0; i < 31; i++) {
		regs[i] = 0;
	}

	debug = false;
}

cpu::~cpu() {

}


uint32_t cpu::fetch(uint32_t addr) {
	return bus.mem.read32(addr);
}
void cpu::execute(uint32_t instr) {
	regs[0] = 0; // $zero

	uint8_t primary = instr >> 26;
	uint8_t secondary = instr & 0x3f;
	if (jump != 0) {	// check for jump branch delay slot
		pc = jump - 4;
		jump = 0;
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
					if(debug) printf("sll 0x%.2X, 0x%.2x, 0x%.8x\n", rt, rd, shift_imm);
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
					if (debug) printf("srl 0x%.2X, 0x%.2x, 0x%.8x\n", rt, rd, shift_imm);
					break;
				}
				case(0x03): { // sra
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint8_t shift_imm = (instr >> 6) & 0x1f;
					regs[rd] = int32_t(regs[rt]) >> shift_imm;
					pc += 4;
					if (debug) printf("sra 0x%.2X, 0x%.2x, 0x%.8x\n", rt, rd, shift_imm);
					break;
				}
				case(0x08): {	// jr
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					jump = regs[rs];
					if (debug) printf("jr 0x%.8x\n", regs[rs]);
					pc += 4;
					break;
				}
				case(0x09): { // jalr
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					jump = regs[rs];
					regs[rd] = pc + 8;
					if (debug) printf("jalr 0x%.8x\n", regs[rs]);
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
			case(0x10): {	
				switch (secondary & 0x0f) {
				case(0x00): { // mfhi
					uint8_t rd = (instr >> 11) & 0x1f;
					regs[rd] = hi;
					if (debug) printf("mfhi 0x%.2X", rd);
					pc += 4;
					break;
				}
				case(0x02): { // mflo
					uint8_t rd = (instr >> 11) & 0x1f;
					regs[rd] = lo;
					if(debug) printf("mflo 0x%.2X", rd);
					pc += 4;
					break;
				}
				case(0x0A): { // div
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					
					int32_t n = int32_t(regs[rs]);
					int32_t d = int32_t(regs[rt]);
					
					hi = uint32_t(n % d);
					lo = uint32_t(n / d);
					if(debug) printf("div 0x%.2X, 0x%.2X\n", rs, rt);
					pc += 4;
					break;
				}
				case(0x0B): { // divu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;

					hi = regs[rs] % regs[rt];
					lo = regs[rs] / regs[rt];
					if (debug) printf("divu 0x%.2X, 0x%.2X\n", rs, rt);
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
					if (debug) printf("addi 0x%.2x, 0x%.2x, 0x%.4x", rt, rs, imm);
					uint32_t result = regs[rs] + regs[rt];
					if (((regs[rs] >> 31) == (regs[rt] >> 31)) && ((regs[rs] >> 31) != (result >> 31))) {
						if (debug) printf(" add exception");
						pc += 4;
						break;
					}
					if (debug) printf("\n");
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
					if (debug) printf("subu 0x%.2x, 0x%.2x, 0x%.2x\n", rs, rd, rt);
					pc += 4;
					break;
				}
				case(0x04): {	// and
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = regs[rs] & regs[rt];
					if (debug) printf("and 0x%.2x, 0x%.8x, 0x%.8x\n", rd, rs, rt);
					pc += 4;
					break;
				}
				case(0x05): {	// or
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = regs[rs] | regs[rt];
					if(debug) printf("or 0x%.2x, 0x%.8x, 0x%.8x\n", rd, rs, rt);
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
					if(debug) printf("addu 0x%.8x, 0x%.8x, 0x%.8x\n", rd, rs, rt);
					break;
				}
				case(0x0A): { // slt
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					printf("slt 0x%.2x, 0x%.8x, 0x%.8x\n", rd, rs, rt);
					regs[rd] = 0;
					if (int32_t(regs[rs]) < int32_t(regs[rt])) {
						regs[rd] = 1;
					}
					pc += 4;
					break;
				}
				case(0x0B): {	// sltu
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					uint16_t imm = instr & 0xffff;
					if(debug) printf("sltu 0x%.2x, 0x%.8x, 0x%.8x\n", rd, rs, rt);
					regs[rd] = 0;
					if (regs[rs] < regs[rt]) {
						regs[rd] = 1;
					}
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
			if (bit16 == 0 ) {		// bltz
				if (bit20 == 1) regs[0x1f] = pc + 4; // check if link (bltzal)
				if (debug) printf("BxxZ 0x%.2x, 0x%.4x", rs, sign_extended_imm);
				if (signed_rs < 0) {
					jump = (pc + 4) + (sign_extended_imm << 2);
					
					if (debug) printf(" branched\n");
				}
				else { if (debug) printf("\n"); }
				pc += 4;
				break;
			}

			if (bit16 == 1) {		// bgez
				if (debug) printf("BxxZ 0x%.2x, 0x%.4x", rs, sign_extended_imm);
				if (bit20 == 1) regs[0x1f] = pc + 4; // check if link (bgezal)
				if (signed_rs >= 0) {
					jump = (pc + 4) + (sign_extended_imm << 2);
					
					if (debug) printf(" branched\n");
				}
				else { if (debug) printf("\n"); }
				pc += 4;
				break;
			}
		}
		case(0x02): { // j
			uint32_t jump_imm = instr & 0x3ffffff;
			jump = (pc & 0xf0000000) | (jump_imm << 2);
			if(debug) printf("j 0x%.8x\n", jump_imm);
			pc += 4;
			break;
		}
		case(0x03): {	// jal
			uint32_t jump_imm = instr & 0x3ffffff;
			jump = ((pc) & 0xf0000000) | (jump_imm << 2);
			if (debug) printf("jal 0x%.8x\n", jump_imm);
			regs[0x1f] = pc + 8;
			pc += 4;
			break;
		}
		case(0x04): {	// beq
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			if (debug) printf("beq 0x%.2x, 0x%.2x, 0x%.4x", rs, rt, sign_extended_imm);
			if (regs[rs] == regs[rt]) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				if (debug) printf(" branched\n");
			}
			else { if (debug) printf("\n"); }
			pc += 4;
			break;
		}
		case(0x05): {	// bne
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			if(debug) printf("bne 0x%.2x, 0x%.2x, 0x%.4x", rs, rt, sign_extended_imm);
			if (regs[rs] != regs[rt]) {
				jump = (pc + 4) + (sign_extended_imm << 2); 
				if(debug) printf(" branched\n");
			}
			else { if(debug) printf("\n"); }
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
			if (debug) printf("blez 0x%.2x, 0x%.4x", rs, sign_extended_imm);
			if (signed_rs <= 0) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				if (debug) printf(" branched\n");
			}
			else { if (debug) printf("\n"); }
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
			if (debug) printf("bgtz 0x%.2x, 0x%.4x", rs, sign_extended_imm);
			if (signed_rs > 0) {
				jump = (pc + 4) + (sign_extended_imm << 2);
				if (debug) printf(" branched\n");
			}
			else { if (debug) printf("\n"); }
			pc += 4;
			break;
		}
		case(0x08): { // addi
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;	
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			if(debug) printf("addi 0x%.2x, 0x%.2x, 0x%.4x", rt, rs, imm);
			uint32_t result = regs[rs] + sign_extended_imm;
			if (((regs[rs] >> 31) == (sign_extended_imm >> 31)) && ((regs[rs] >> 31) != (result >> 31))) {
				if (debug) printf(" addi exception");
				pc += 4;
				break;
			}
			if(debug) printf("\n");
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
			if(debug) printf("addiu 0x%.2x, 0x%.2x, 0x%.4x\n", rt, rs, imm);
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
			printf("slti 0x%.2X, 0x%.2X, 0x%.4X\n", rt, rs, imm);
			break;
		}
		case(0x0B): { // sltiu
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			int32_t signed_sign_extended_imm = int32_t(uint32_t(int32_t(int16_t(imm))));

			
			regs[rt] = 0;
			if (regs[rs] < signed_sign_extended_imm)
				regs[rt] = 1;
			pc += 4;
			printf("sltiu 0x%.2X, 0x%.2X, 0x%.4X\n", rt, rs, imm);
			break;
		}
		case(0x0C): {	// andi
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			if (debug) printf("andi 0x%.2X, 0x%.2X, 0x%.4x\n", rs, rt, imm);
			regs[rt] = (regs[rs] & imm);
			pc += 4;
			break;
		}
		case(0x0D): {	// ori
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			if(debug) printf("ori 0x%.2X, 0x%.2X, 0x%.4x\n", rs, rt, imm);
			regs[rt] = (regs[rs] | imm);
			pc += 4;
			break;
		}
		case(0x0E): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0F): {	// lui
			uint8_t rt = (instr >> 16) & 0x1f;
			uint32_t imm = instr & 0xffff;
			if(debug) printf("lui 0x%.2X, 0x%.4x\n", rt, imm);
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
				printf("mfc0 0x%.2x, 0x%.2x\n", rt, rd);
				break;
			}
			case(0b00100): { // mtc0
				uint8_t rs = (instr >> 21) & 0x1f;
				uint8_t rd = (instr >> 11) & 0x1f;
				uint8_t rt = (instr >> 16) & 0x1f;
				uint16_t imm = instr & 0xffff;
				COP0.regs[rd] = regs[rt];
				printf("mtc0 0x%.2x, 0x%.2x\n", rt, rd);
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
			if (debug) printf("lb 0x%.2X, 0x%.4x(0x%.2x)", rt, imm, rs);
			if ((COP0.regs[12] & 0x10000) == 0) {
				uint8_t byte = bus.mem.read(addr);
				regs[rt] = uint32_t(int32_t(int8_t(byte)));
				if (debug) printf("\n");
			}
			else {
				if (debug) printf(" cache isolated, ignoring load\n");
			}
			pc += 4;
			break;
		}
		case(0x01): {
			if (debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x03): {	// lw  
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			if(debug) printf("lw 0x%.2X, 0x%.4x(0x%.2x)", rt, imm, regs[rs]);
			if ((COP0.regs[12] & 0x10000) == 0) {
				regs[rt] = bus.mem.read32(addr);
				if(debug) printf("\n");
			}
			else {
				if(debug) printf(" cache isolated, ignoring load\n");
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
			if (debug) printf("lbu 0x%.2X, 0x%.4x(0x%.2x)", rt, imm, regs[rs]);
			if ((COP0.regs[12] & 0x10000) == 0) {
				uint8_t byte = bus.mem.read(addr);
				regs[rt] = byte;
				if (debug) printf("\n");
			}
			else {
				if (debug) printf(" cache isolated, ignoring load\n");
			}
			pc += 4;
			break;
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
		case(0x08): {	// sb
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			if (debug) printf("sb 0x%.2X, 0x%.4x(0x%.2x)", rt, imm, rs);
			if ((COP0.regs[12] & 0x10000) == 0) {
				bus.mem.write(addr, uint8_t(regs[rt]), true);
				if (debug) printf("\n");
			}
			else {
				if (debug) printf(" cache isolated, ignoring write\n");
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
			if (debug) printf("sh 0x%.2X, 0x%.4x(0x%.2x)", rt, imm, rs);
			if ((COP0.regs[12] & 0x10000) == 0) {
				bus.mem.write16(addr, uint16_t(regs[rt]));
				if (debug) printf("\n");
			}
			else {
				if (debug) printf(" cache isolated, ignoring write\n");
			}
			pc += 4;
			break;
			
		}
		case(0x0A): {
			printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): { // sw
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = regs[rs] + sign_extended_imm;
			if(debug) printf("sw 0x%.2X, 0x%.4x(0x%.2x)", rt, imm, rs);
			if ((COP0.regs[12] & 0x10000) == 0) {
				bus.mem.write32(addr, regs[rt]);
				if(debug) printf("\n");
			} else {
				if(debug) printf(" cache isolated, ignoring write\n");
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
	

}