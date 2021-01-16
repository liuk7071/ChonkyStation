#include "cpu.h"
#include <iostream>
cpu::cpu() {
	bus.mem.loadBios();
	pc = 0xbfc00000;
}

cpu::~cpu() {

}

uint32_t cpu::fetch(uint32_t addr) {
	return bus.mem.read32(addr);
}
void cpu::execute(uint32_t instr) {
	uint8_t primary = instr >> 26;
	switch (primary & 0xf0) {
	case(0x00): {
		switch (primary & 0x0f) {
		case(0x00): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x01): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x03): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x04): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x05): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x06): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x07): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x08): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x09): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0A): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0B): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0C): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0D): {
			uint8_t rs = (instr >> 21) & 0x1f;
			uint8_t rt = (instr >> 16) & 0x1f;
			uint32_t imm = instr & 0xffff;
			printf("ori 0x%.2X, 0x%.2X, 0x%.4x\n", rs, rt, imm);
			regs[rt] = (regs[rs] | imm);
			pc += 4;
			break;
		}
		case(0x0E): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0F): {	
			uint8_t rt = (instr >> 16) & 0x1f;
			uint32_t imm = instr & 0xffff;
			printf("lui 0x%.2X, 0x%.4x\n", rt, imm);
			imm <<= 16;
			regs[rt] = imm;
			pc += 4;
			break;
		}
		default:
			printf("Unknown instruction\n");
			exit(0);
		}
		break;
		}
	
	case(0x10): {
		switch (primary & 0x0f) {
		case(0x00): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x01): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x03): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x04): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x05): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x06): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x07): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x08): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x09): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0A): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0B): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0C): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0D): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0E): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0f): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		default:
			printf("Unknown instruction\n");
			exit(0);
		}
		break;
		}
	
	case(0x20): {
		switch (primary & 0x0f) {
		case(0x00): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x01): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x03): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x04): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x05): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x06): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x07): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x08): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x09): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0A): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0B): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0C): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0D): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0E): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0f): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		default:
			printf("Unknown instruction\n");
			exit(0);
		}
		break;
		}
	
	case(0x30): {
		switch (primary & 0x0f) {
		case(0x00): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x01): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x02): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x03): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x04): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x05): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x06): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x07): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x08): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x09): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0A): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0B): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0C): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0D): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0E): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		case(0x0f): {
			printf("Unknown instruction\n"); exit(0); break;
		}
		default:
			printf("Unknown instruction\n");
			exit(0);
		}
		printf("Unknown instruction\n"); exit(0); break;
		}
	

	

	default: {
		printf("Unknown instruction\n");
		exit(0);
	}
	}

}