#include "cop0.h"
cop0::cop0() {

}
cop0::~cop0() {

}

void cop0::execute(uint32_t instr, uint32_t* regs) {
	uint8_t cop_instr = instr >> 21;
	switch (cop_instr) {
	case(0b00100): { // mtc0
		uint8_t rs = (instr >> 21) & 0x1f;
		uint8_t rd = (instr >> 11) & 0x1f;
		uint8_t rt = (instr >> 16) & 0x1f;
		uint16_t imm = instr & 0xffff;
		switch (rd) {
		case 3: {
			bpc = regs[rt];
			break;
		}
		case 5: {
			bda = regs[rt];
			break;
		}
		case 6: {
			i = regs[rt];
			break;
		}
		case 7: {
			dcic = regs[rt];
			break;
		}
		case 9: {
			bdam = regs[rt];
			break;
		}
		case 11: {
			bpcm = regs[rt];
			break;
		}
		case 12: {
			sr = regs[rt];
			break;
		}
		case 13: {
			cause = regs[rt];
			break;
		}
		default: {
			printf("Unhandled cop0 register");
			exit(0);
		}
		}
		printf("mtc0 0x%.2x, 0x%.2x\n", rt, rd);
		break;
	}
	default:
		printf("Unknown coprocessor instruction.");
		exit(0);
	}
}