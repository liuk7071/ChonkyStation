#include "cpu.h"
#include <iostream>
cpu::cpu() {
	bus.mem.loadBios();
	pc = 0xbfc00000;
	debug = true;
}

cpu::~cpu() {

}

uint32_t cpu::fetch(uint32_t addr) {
	return bus.mem.read32(addr);
}
void cpu::execute(uint32_t instr) {
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
				default: {
					if(debug) printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
				break;
				}
				break;
				}
			case(0x10): {	
				switch (secondary & 0x0f) {
				default: {
					if(debug) printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
				break;
				}
				break;
			}
			case(0x20): {
				switch (secondary & 0x0f) {
				case(0x05): {	// or
					uint8_t rs = (instr >> 21) & 0x1f;
					uint8_t rd = (instr >> 11) & 0x1f;
					uint8_t rt = (instr >> 16) & 0x1f;
					regs[rd] = regs[rs] | regs[rt];
					if(debug) printf("or 0x%.2x, 0x%.8x, 0x%.8x\n", rd, rs, rt);
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
					if(debug) printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
				break;
				}
				break;
			}
			case(0x30): {
				switch (secondary & 0x0f) {
				default: {
					if(debug) printf("Unknown subinstruction: 0x%.2X\n", secondary);
					exit(0);
				}
				break;
				}
				break;
			}

			default: {
				if(debug) printf("Unknown subinstruction: 0x%.2X\n", secondary);
				exit(0);
			}
			break;
			}

			break;
		}
		case(0x01): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): { // j
			uint32_t jump_imm = instr & 0x3ffffff;
			jump = (pc & 0xf0000000) | (jump_imm << 2);
			if(debug) printf("j 0x%.8x\n", jump_imm);
			pc += 4;
			break;
		}
		case(0x03): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x04): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x05): {	// bne
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rd = (instr >> 11) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			if(debug) printf("bne 0x%.2x, 0x%.2x, 0x%.4x", rs, rt, sign_extended_imm);
			if (regs[rs] != regs[rt]) {
				jump = pc + (sign_extended_imm << 2); 
				if(debug) printf(" branched\n");
			}
			else { if(debug) printf("\n"); }
			pc += 4;
			break;
		}
		case(0x06): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x07): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
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
				if(debug) printf(" addi exception");
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
		case(0x0A): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0C): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
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
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
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
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		break;
		}
	
	case(0x10): {
		switch (primary & 0x0f) {
		case(0x00): {	// COP 0
			COP0.execute(instr, regs);
			pc += 4;
			//if(debug) printf("\n");
			break;
		}
		case(0x01): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x03): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x04): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x05): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x06): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x07): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x08): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x09): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0A): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0C): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0D): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0E): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0f): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		default:
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		break;
		}
	
	case(0x20): {
		switch (primary & 0x0f) {
		case(0x00): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x01): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x03): {	// lw  need to do load delay slot
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = rs + sign_extended_imm;
			if(debug) printf("lw 0x%.2X, 0x%.4x(0x%.8x)", rt, imm, regs[rs]);
			if ((COP0.sr & 0x10000) == 0) {
				regs[rt] = bus.mem.read32(addr);
				if(debug) printf("\n");
			}
			else {
				if(debug) printf(" cache isolated, ignoring load\n");
			}

			pc += 4;
			break;
		}
		case(0x04): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x05): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x06): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x07): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x08): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x09): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0A): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): { // sw
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint16_t imm = instr & 0xffff;
			uint32_t sign_extended_imm = uint32_t(int32_t(int16_t(imm)));
			uint32_t addr = rs + sign_extended_imm;
			if(debug) printf("sw 0x%.2X, 0x%.4x(0x%.8x)", rt, imm, regs[rs]);
			if ((COP0.sr & 0x10000) == 0) {
				bus.mem.write(addr, regs[rt]);
				if(debug) printf("\n");
			} else {
				if(debug) printf(" cache isolated, ignoring write\n");
			}
			
			pc += 4;
			break;
		}
		case(0x0C): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0D): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0E): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0f): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		default:
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		break;
		}
	
	case(0x30): {
		switch (primary & 0x0f) {
		case(0x00): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x01): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x02): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x03): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x04): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x05): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x06): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x07): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x08): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x09): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0A): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0B): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0C): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0D): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0E): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		case(0x0f): {
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
		default:
			if(debug) printf("Unknown instruction: 0x%.2X\n", primary);
			exit(0);
		}
		if(debug) printf("Unknown instruction: 0x%.2X\n", primary); exit(0); break;
		}
	

	

	default: {
		if(debug) printf("Unknown instruction: 0x%.2X\n", primary);
		exit(0);
	}
	}
	

}