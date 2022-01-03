#include "gte.h"

void gte::execute(uint32_t instr, uint32_t* gpr) {
	instruction = instr;
	switch (instr & 0x3f) {
	case MOVE: {
		switch ((instr >> 21) & 0x1f) {
		case MFC2: moveMFC2(gpr); break;
		case CFC2: moveCFC2(gpr); break;
		case MTC2: moveMTC2(gpr); break;
		case CTC2: moveCTC2(gpr); break;
		default:
			printf("Unimplemented GTE MOVE instruction: 0x%x\n", (instr >> 21) & 0x1f);
			exit(1);
		}
		break;
	}
	case RTPS: commandRTPS(); break;
	case NCLIP: commandNCLIP(); break;
	case NCDS: commandNCDS(); break;
	case AVSZ3: commandAVSZ3(); break;
	case AVSZ4: commandAVSZ4(); break;
	case RTPT: commandRTPT(); break;
	default:
		printf("Unimplemented GTE instruction: 0x%x\n", instr);
		exit(1);
	}
}

// Commands
void gte::moveMFC2(uint32_t* gpr) {
	switch ((instruction >> 11) & 0x1f) {
	case 24: {
		gpr[(instruction >> 16) & 0x1f] = cop2d[(instruction >> 11) & 0x1f];
		break;
	}
	default:
		printf("Unimplemented MFC2 destination: %d\n", (instruction >> 11) & 0x1f);
		exit(1);
	}
}
void gte::moveMTC2(uint32_t* gpr) {
	switch ((instruction >> 11) & 0x1f) {
	case 8: {
		cop2d[(instruction >> 11) & 0x1f] = gpr[(instruction >> 16) & 0x1f];
		break;
	}
	default:
		printf("Unimplemented MTC2 destination: %d\n", (instruction >> 11) & 0x1f);
		exit(1);
	}
}
void gte::moveCFC2(uint32_t* gpr) {
	gpr[(instruction >> 16) & 0x1f] = cop2c[(instruction >> 11) & 0x1f];
}
void gte::moveCTC2(uint32_t* gpr) {
	switch ((instruction >> 11) & 0x1f) {
	// S16
	case 4:
	case 12:
	case 20:
	case 26:
	case 27:
	case 29:
	case 30: {
		//cop2c[(instruction >> 11) & 0x1f] = (gpr[(instruction >> 16) & 0x1f] & 0xffff);
		cop2c[(instruction >> 11) & 0x1f] = (uint32_t)(int16_t)(gpr[(instruction >> 16) & 0x1f]);
		break;
	}
	// 32
	case 0:
	case 1:
	case 2:
	case 3:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 28: {
		cop2c[(instruction >> 11) & 0x1f] = gpr[(instruction >> 16) & 0x1f];
		break;
	}

	default:
		printf("Unimplemented CTC2 destination: %d\n", (instruction >> 11) & 0x1f);
		exit(1);
	}
}

void gte::commandRTPS() {
	MAC1 = int32_t(((TRX * 0x1000) + (RT11 * VX0) + (RT12 * VY0) + (RT13 * VZ0)) >> sf(instruction));
	MAC2 = int32_t(((TRY * 0x1000) + (RT21 * VX0) + (RT22 * VY0) + (RT23 * VZ0)) >> sf(instruction));
	MAC3 = int32_t(((TRZ * 0x1000) + (RT31 * VX0) + (RT32 * VY0) + (RT33 * VZ0)) >> sf(instruction));
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
	SZ3 = (int32_t(MAC3) >> ((1 - sf(instruction) * 12)));
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR1) + OFX; SETSX2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR2) + OFY; SETSY2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * DQA) + DQB; IR0 = (MAC0 / 0x1000);
}

void gte::commandNCLIP() {
	MAC0 = (SX0 * SY1) + (SX1 * SY2) + (SX2 * SY0) - (SX0 * SY2) - (SX1 * SY0) - (SX2 * SY1);
}

void gte::commandNCDS() {
	MAC1 = (int32_t((L11 * VX0) + (L12 * VY0) + (L13 * VZ0)) >> (sf(instruction) * 12));
	MAC2 = (int32_t((L21 * VX0) + (L22 * VY0) + (L23 * VZ0)) >> (sf(instruction) * 12));
	MAC3 = (int32_t((L31 * VX0) + (L32 * VY0) + (L33 * VZ0)) >> (sf(instruction) * 12));
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
	MAC1 = (int32_t((RBK * 0x1000) + ((LR1 * IR1) + (LR2 * IR2) + (LR3 * IR3))) >> (sf(instruction) * 12));
	MAC2 = (int32_t((GBK * 0x1000) + ((LG1 * IR1) + (LG2 * IR2) + (LG3 * IR3))) >> (sf(instruction) * 12));
	MAC1 = (int32_t((BBK * 0x1000) + ((LB1 * IR1) + (LB2 * IR2) + (LB3 * IR3))) >> (sf(instruction) * 12));
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
	MAC1 = ((R * IR1) << 4);
	MAC2 = ((G * IR2) << 4);
	MAC3 = ((B * IR3) << 4);
	MAC1 = (MAC1 + ((RFC - MAC1) * IR0));
	MAC2 = (MAC2 + ((GFC - MAC2) * IR0));
	MAC3 = (MAC3 + ((BFC - MAC3) * IR0));
	MAC1 = int32_t(MAC1) >> (sf(instruction) * 12);
	MAC2 = int32_t(MAC2) >> (sf(instruction) * 12);
	MAC3 = int32_t(MAC3) >> (sf(instruction) * 12);
	uint32_t col = ((((MAC1) / 16) << 0) | (((MAC2) / 16) << 8) | (((MAC3) / 16) << 16) | (CD2 << 24));
	RGB2 = col;
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
}

void gte::commandAVSZ3() {
	MAC0 = ZSF3 * (SZ1 + SZ2 + SZ3);
	OTZ = (MAC0 / 0x1000);
}

void gte::commandAVSZ4() {
	MAC0 = ZSF4 * (SZ0 + SZ1 + SZ2 + SZ3);
	OTZ = (MAC0 / 0x1000);
}

void gte::commandRTPT() {
	MAC1 = int32_t(((TRX * 0x1000) + (RT11 * VX0) + (RT12 * VY0) + (RT13 * VZ0)) >> sf(instruction));
	MAC2 = int32_t(((TRY * 0x1000) + (RT21 * VX0) + (RT22 * VY0) + (RT23 * VZ0)) >> sf(instruction));
	MAC3 = int32_t(((TRZ * 0x1000) + (RT31 * VX0) + (RT32 * VY0) + (RT33 * VZ0)) >> sf(instruction));
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
	SZ3 = (int32_t(MAC3) >> ((1 - sf(instruction) * 12)));
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR1) + OFX; SETSX2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR2) + OFY; SETSY2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * DQA) + DQB; IR0 = (MAC0 / 0x1000);

	MAC1 = int32_t(((TRX * 0x1000) + (RT11 * VX0) + (RT12 * VY0) + (RT13 * VZ0)) >> sf(instruction));
	MAC2 = int32_t(((TRY * 0x1000) + (RT21 * VX0) + (RT22 * VY0) + (RT23 * VZ0)) >> sf(instruction));
	MAC3 = int32_t(((TRZ * 0x1000) + (RT31 * VX0) + (RT32 * VY0) + (RT33 * VZ0)) >> sf(instruction));
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
	SZ3 = (int32_t(MAC3) >> ((1 - sf(instruction) * 12)));
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR1) + OFX; SETSX2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR2) + OFY; SETSY2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * DQA) + DQB; IR0 = (MAC0 / 0x1000);

	MAC1 = int32_t(((TRX * 0x1000) + (RT11 * VX0) + (RT12 * VY0) + (RT13 * VZ0)) >> sf(instruction));
	MAC2 = int32_t(((TRY * 0x1000) + (RT21 * VX0) + (RT22 * VY0) + (RT23 * VZ0)) >> sf(instruction));
	MAC3 = int32_t(((TRZ * 0x1000) + (RT31 * VX0) + (RT32 * VY0) + (RT33 * VZ0)) >> sf(instruction));
	IR1 = MAC1; IR2 = MAC2; IR3 = MAC3;
	SZ3 = (int32_t(MAC3) >> ((1 - sf(instruction) * 12)));
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR1) + OFX; SETSX2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * IR2) + OFY; SETSY2(MAC0 / 0x10000);
	MAC0 = (((((H * 0x20000) / SZ3) + 1) / 2) * DQA) + DQB; IR0 = (MAC0 / 0x1000);
}