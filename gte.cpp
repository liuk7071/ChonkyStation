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
	case RTPS: cop2c[31] = 0; commandRTPS(); break;
	case NCLIP: cop2c[31] = 0; commandNCLIP(); break;
	case OP: cop2c[31] = 0; commandOP(); break;
	case DPCS: cop2c[31] = 0; commandDPCS(); break;
	case INTPL: cop2c[31] = 0; commandINTPL(); break;
	case MVMVA: cop2c[31] = 0; commandMVMVA(); break;
	case NCDS: cop2c[31] = 0; commandNCDS(); break;
	case NCDT: cop2c[31] = 0; commandNCDT(); break;
	case NCCS: cop2c[31] = 0; commandNCCS(); break;
	case NCS: cop2c[31] = 0; commandNCS(); break;
	case NCT: cop2c[31] = 0; commandNCT(); break;
	case SQR: cop2c[31] = 0; commandSQR(); break;
	case AVSZ3: cop2c[31] = 0; commandAVSZ3(); break;
	case AVSZ4: cop2c[31] = 0; commandAVSZ4(); break;
	case RTPT: cop2c[31] = 0; commandRTPT(); break;
	case GPF: cop2c[31] = 0; commandGPF(); break;
	case GPL: cop2c[31] = 0; commandGPL(); break;
	default:
		printf("Unimplemented GTE instruction: 0x%x\n", instr);
		exit(1);
	}
}

uint32_t gte::mx(int x, int i) {
	switch (x) {
	case 0: // Rotation matrix
		switch (i) {
		case 11: return RT11;
		case 12: return RT12;
		case 13: return RT13;
		case 21: return RT21;
		case 22: return RT22;
		case 23: return RT23;
		case 31: return RT31;
		case 32: return RT32;
		case 33: return RT33;
		default:
			printf("Bad matrix index (%d)\n", i); exit(0);
		}
	case 1: // Light matrix
		switch (i) {
		case 11: return L11;
		case 12: return L12;
		case 13: return L13;
		case 21: return L21;
		case 22: return L22;
		case 23: return L23;
		case 31: return L31;
		case 32: return L32;
		case 33: return L33;
		default:
			printf("Bad matrix index (%d)\n", i); exit(0);
		}
	case 2: // Colour matrix
		switch (i) {
		case 11: return LR1;
		case 12: return LR2;
		case 13: return LR3;
		case 21: return LG1;
		case 22: return LG2;
		case 23: return LG3;
		case 31: return LB1;
		case 32: return LB2;
		case 33: return LB3;
		default:
			printf("Bad matrix index (%d)\n", i); exit(0);
		}
	default:
		printf("Bad mx value (%d)\n", x);
		exit(0);
	}
}
uint32_t gte::vx(int x, int i) {
	switch (x) {
	case 0:
		switch (i) {
		case 0: return VX0;
		case 1: return VY0;
		case 2: return VZ0;
		default:
			printf("Bad vector index (%d)\n", i);
			exit(0);
		}
	case 1:
		switch (i) {
		case 0: return VX1;
		case 1: return VY1;
		case 2: return VZ1;
		default:
			printf("Bad vector index (%d)\n", i);
			exit(0);
		}
	case 2:
		switch (i) {
		case 0: return VX2;
		case 1: return VY2;
		case 2: return VZ2;
		default:
			printf("Bad vector index (%d)\n", i);
			exit(0);
		}
	case 3:
		switch (i) {
		case 0: return IR1;
		case 1: return IR2;
		case 2: return IR3;
		default:
			printf("Bad vector index (%d)\n", i);
			exit(0);
		}
	default:
		printf("Bad vx value (%d)\n", x);
		exit(0);
	}
}
uint32_t gte::tx(int x, int i) {
	switch (x) {
	case 0:
		switch (i) {
		case 0: return TRX;
		case 1: return TRY;
		case 2: return TRZ;
		default:
			printf("Bad vector index (%d)\n", i);
			exit(0);
		}
	case 1:
		switch (i) {
		case 0: return RBK;
		case 1: return GBK;
		case 2: return BBK;
		default:
			printf("Bad vector index (%d)\n", i);
			exit(0);
		}
	default:
		printf("Bad tx value (%d)\n", x);
		exit(0);
	}
}

// Helpers
uint32_t gte::readCop2d(uint32_t reg) {
	switch (reg) {
	case 1:
	case 3:
	case 5:
	case 8:
	case 9:
	case 10:
	case 11:
		return (uint32_t)(int16_t)cop2d[reg];
	
	case 7:
	case 16:
	case 17:
	case 18:
	case 19:
		return cop2d[reg] & 0xffff;

	case 15: // SXYP returns SXY2
		return cop2d[14];
	
	default:
		return cop2d[reg];
	}

	/*switch (reg) {
	case 12:
	case 13:
	case 14:
	case 22: {
		return cop2d[reg];
	}
	case 8: {
		return (uint32_t)(int16_t)(cop2d[reg]);
	}
	default:
		printf("Unhandled cop2d read %d\n", reg);
		exit(1);
	}*/
}
void gte::writeCop2d(uint32_t reg, uint32_t value) {
	switch (reg) {
	case 0: 
	case 2:
	case 4:
	case 6: {
		cop2d[reg] = value;
		break;
	}
	case 1:
	case 3:
	case 5: {
		cop2d[reg] = (uint32_t)(int16_t)(value);
		break;
	}
	default:
		cop2d[reg] = value;
		break;
	}
}

void gte::writeCop2c(uint32_t reg, uint32_t value) {
	switch (reg) {
	case 4:
	case 12:
	case 20:
	case 26: 
	case 27:
	case 29:
	case 30:
		cop2c[reg] = (uint32_t)(int16_t)value;
		break;
	default:
		cop2c[reg] = value;
		break;
	}
}

// Push a Z value to the Z-coordinate FIFO
void gte::pushZ(uint16_t value) {
	SZ0 = SZ1;
	SZ1 = SZ2;
	SZ2 = SZ3;
	SZ3 = value;
}

void gte::pushColour() {
	RGB0 = RGB1;
	RGB1 = RGB2;

	const uint32_t col = (saturate(((int32_t)MAC1 >> 4), 0, 0xff) << 0) | (saturate(((int32_t)MAC2 >> 4), 0, 0xff) << 8) | (saturate(((int32_t)MAC3 >> 4), 0, 0xff) << 16) | (CD2 << 24);
	//RGB2 = 0x00345678 | (CD2 << 24);
	RGB2 = col;
}

void gte::setIRFromMAC(bool lm) {
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
}

// Commands
void gte::moveMFC2(uint32_t* gpr) {
	//printf("mfc2 (r%d <- cop2r%d)\n", (instruction >> 16) & 0x1f, (instruction >> 11) & 0x1f);
	gpr[(instruction >> 16) & 0x1f] = readCop2d((instruction >> 11) & 0x1f);
	/*switch ((instruction >> 11) & 0x1f) {
	case 7: {
		//printf("cop2r%d (0x%x) -> r%d\n", (instruction >> 11) & 0x1f, cop2d[(instruction >> 11) & 0x1f], (instruction >> 16) & 0x1f);
		gpr[(instruction >> 16) & 0x1f] = cop2d[(instruction >> 11) & 0x1f];
		break;
	}
	case 24: {
		//printf("cop2r%d (0x%x) -> r%d\n", (instruction >> 11) & 0x1f, cop2d[(instruction >> 11) & 0x1f], (instruction >> 16) & 0x1f);
		gpr[(instruction >> 16) & 0x1f] = cop2d[(instruction >> 11) & 0x1f];
		//gpr[(instruction >> 16) & 0x1f] = 200;
		break;
	}
	default:
		printf("Unimplemented MFC2 destination: %d\n", (instruction >> 11) & 0x1f);
		//exit(1);
	}*/
}
void gte::moveMTC2(uint32_t* gpr) {
	//printf("mtc2 (cop2r%d <- r%d)\n", (instruction >> 11) & 0x1f, (instruction >> 16) & 0x1f);
	writeCop2d((instruction >> 11) & 0x1f, gpr[(instruction >> 16) & 0x1f]);
	/*switch ((instruction >> 11) & 0x1f) {
	case 8: {
		//printf("0x%x -> cop2r%d\n", (uint32_t)(int16_t)gpr[(instruction >> 16) & 0x1f], (instruction >> 11) & 0x1f);
		cop2d[(instruction >> 11) & 0x1f] = (uint32_t)(int16_t)(gpr[(instruction >> 16) & 0x1f]);
		break;
	}
	default:
		printf("Unimplemented MTC2 destination: %d\n", (instruction >> 11) & 0x1f);
		//exit(1);
	}*/
}
void gte::moveCFC2(uint32_t* gpr) {
	//printf("cfc2 (r%d <- cnt%d)\n", (instruction >> 16) & 0x1f, (instruction >> 11) & 0x1f);
	//printf("cnt%d (0x%x) -> r%d\n", (instruction >> 11) & 0x1f, cop2c[(instruction >> 11) & 0x1f], (instruction >> 16) & 0x1f);
	gpr[(instruction >> 16) & 0x1f] = cop2c[(instruction >> 11) & 0x1f];
}
void gte::moveCTC2(uint32_t* gpr) {
	//printf("ctc2 (cnt%d <- r%d)\n", (instruction >> 11) & 0x1f, (instruction >> 16) & 0x1f);
	writeCop2c((instruction >> 11) & 0x1f, gpr[(instruction >> 16) & 0x1f]);
}

void gte::commandRTPS() {
	//printf("rtps\n");
	const int lm = 0;
	const int shift = sf(instruction) * 12;
	MAC1 = int64_t(((int64_t)(int32_t)TRX * 0x1000) + ((int16_t)RT11 * (int16_t)VX0) + ((int16_t)RT12 * (int16_t)VY0) + ((int16_t)RT13 * (int16_t)VZ0)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)TRY * 0x1000) + ((int16_t)RT21 * (int16_t)VX0) + ((int16_t)RT22 * (int16_t)VY0) + ((int16_t)RT23 * (int16_t)VZ0)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)TRZ * 0x1000) + ((int16_t)RT31 * (int16_t)VX0) + ((int16_t)RT32 * (int16_t)VY0) + ((int16_t)RT33 * (int16_t)VZ0)) >> shift;
	setIRFromMAC(lm);
	auto newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//uint32_t _proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	//int32_t _proj_factor = ((H * 0x20000) / (uint16_t)SZ3);
	int32_t _proj_factor = gte_divide(H, SZ3);
	int64_t proj_factor = (int64_t)(_proj_factor);
	int64_t _x = (int64_t)(int16_t)(IR1);
	int64_t _y = (int64_t)(int16_t)(IR2);
	int64_t x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	int64_t y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	x = saturate((x >> 16), -0x400, 0x3ff);
	y = saturate((y >> 16), -0x400, 0x3ff);
	SETSX2(x);
	SETSY2(y);
	SXY2 = (y << 16) | x;
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	int64_t depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = saturate((int16_t)(depth), 0, 0x1000);
}

void gte::commandDPCS() {
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = (uint32_t)R << 16;
	MAC2 = (uint32_t)G << 16;
	MAC3 = (uint32_t)B << 16;

	// Interpolate colour
	uint32_t _MAC1 = MAC1;
	uint32_t _MAC2 = MAC2;
	uint32_t _MAC3 = MAC3;
	MAC1 = (int32_t)((((int64_t)RFC << 12) - (int32_t)MAC1) >> shift);
	MAC2 = (int32_t)((((int64_t)GFC << 12) - (int32_t)MAC2) >> shift);
	MAC3 = (int32_t)((((int64_t)BFC << 12) - (int32_t)MAC3) >> shift);
	IR1 = (int16_t)saturate(MAC1, -0x8000, 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000, 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000, 0x7fff);
	MAC1 = (int32_t)(((int64_t)IR1 * IR0) + (int32_t)_MAC1) >> shift;
	MAC2 = (int32_t)(((int64_t)IR2 * IR0) + (int32_t)_MAC2) >> shift;
	MAC3 = (int32_t)(((int64_t)IR3 * IR0) + (int32_t)_MAC3) >> shift;
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandINTPL() {
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = (int32_t)IR1 << 12;
	MAC2 = (int32_t)IR2 << 12;
	MAC3 = (int32_t)IR3 << 12;

	// Interpolate colour
	uint32_t _MAC1 = MAC1;
	uint32_t _MAC2 = MAC2;
	uint32_t _MAC3 = MAC3;
	MAC1 = (int32_t)((((int64_t)RFC << 12) - (int32_t)MAC1) >> shift);
	MAC2 = (int32_t)((((int64_t)GFC << 12) - (int32_t)MAC2) >> shift);
	MAC3 = (int32_t)((((int64_t)BFC << 12) - (int32_t)MAC3) >> shift);
	IR1 = (int16_t)saturate(MAC1, -0x8000, 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000, 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000, 0x7fff);
	MAC1 = (int32_t)(((int64_t)IR1 * IR0) + (int32_t)_MAC1) >> shift;
	MAC2 = (int32_t)(((int64_t)IR2 * IR0) + (int32_t)_MAC2) >> shift;
	MAC3 = (int32_t)(((int64_t)IR3 * IR0) + (int32_t)_MAC3) >> shift;
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandMVMVA() {
	const int lm = this->lm(instruction);
	const int shift = sf(instruction) * 12;
	const int m = this->m(instruction);
	const int v = this->v(instruction);
	const int cv = this->cv(instruction);
	MAC1 = int64_t(((int64_t)(int32_t)tx(m, 0) * 0x1000) + ((int16_t)mx(m, 11) * (int16_t)vx(v, 0)) + ((int16_t)mx(m, 12) * (int16_t)vx(v, 1)) + ((int16_t)mx(m, 13) * (int16_t)vx(v, 2))) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)tx(m, 1) * 0x1000) + ((int16_t)mx(m, 21) * (int16_t)vx(v, 0)) + ((int16_t)mx(m, 22) * (int16_t)vx(v, 1)) + ((int16_t)mx(m, 23) * (int16_t)vx(v, 2))) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)tx(m, 2) * 0x1000) + ((int16_t)mx(m, 31) * (int16_t)vx(v, 0)) + ((int16_t)mx(m, 32) * (int16_t)vx(v, 1)) + ((int16_t)mx(m, 33) * (int16_t)vx(v, 2))) >> shift;
	setIRFromMAC(lm);
}

void gte::commandNCDS() {
	//printf("ncds\n");
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX0) + (int64_t)((int16_t)L12 * (int16_t)VY0) + (int64_t)((int16_t)L13 * (int16_t)VZ0)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX0) + (int64_t)((int16_t)L22 * (int16_t)VY0) + (int64_t)((int16_t)L23 * (int16_t)VZ0)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX0) + (int64_t)((int16_t)L32 * (int16_t)VY0) + (int64_t)((int16_t)L33 * (int16_t)VZ0)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = ((int64_t)R * (int16_t)IR1) << 4;
	MAC2 = ((int64_t)G * (int16_t)IR2) << 4;
	MAC3 = ((int64_t)B * (int16_t)IR3) << 4;

	// Interpolate colour
	uint32_t _MAC1 = MAC1;
	uint32_t _MAC2 = MAC2;
	uint32_t _MAC3 = MAC3;
	MAC1 = (int32_t)((((int64_t)RFC << 12) - (int32_t)MAC1) >> shift);
	MAC2 = (int32_t)((((int64_t)GFC << 12) - (int32_t)MAC2) >> shift);
	MAC3 = (int32_t)((((int64_t)BFC << 12) - (int32_t)MAC3) >> shift);
	IR1 = (int16_t)saturate(MAC1, -0x8000, 0x7fff);	
	IR2 = (int16_t)saturate(MAC2, -0x8000, 0x7fff);	
	IR3 = (int16_t)saturate(MAC3, -0x8000, 0x7fff);
	MAC1 = (int32_t)(((int64_t)IR1 * IR0) + (int32_t)_MAC1) >> shift;
	MAC2 = (int32_t)(((int64_t)IR2 * IR0) + (int32_t)_MAC2) >> shift;
	MAC3 = (int32_t)(((int64_t)IR3 * IR0) + (int32_t)_MAC3) >> shift;
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandNCDT() {
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX0) + (int64_t)((int16_t)L12 * (int16_t)VY0) + (int64_t)((int16_t)L13 * (int16_t)VZ0)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX0) + (int64_t)((int16_t)L22 * (int16_t)VY0) + (int64_t)((int16_t)L23 * (int16_t)VZ0)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX0) + (int64_t)((int16_t)L32 * (int16_t)VY0) + (int64_t)((int16_t)L33 * (int16_t)VZ0)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = ((int64_t)R * (int16_t)IR1) << 4;
	MAC2 = ((int64_t)G * (int16_t)IR2) << 4;
	MAC3 = ((int64_t)B * (int16_t)IR3) << 4;

	// Interpolate colour
	uint32_t _MAC1 = MAC1;
	uint32_t _MAC2 = MAC2;
	uint32_t _MAC3 = MAC3;
	MAC1 = (int32_t)((((int64_t)RFC << 12) - (int32_t)MAC1) >> shift);
	MAC2 = (int32_t)((((int64_t)GFC << 12) - (int32_t)MAC2) >> shift);
	MAC3 = (int32_t)((((int64_t)BFC << 12) - (int32_t)MAC3) >> shift);
	IR1 = (int16_t)saturate(MAC1, -0x8000, 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000, 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000, 0x7fff);
	MAC1 = (int32_t)(((int64_t)IR1 * IR0) + (int32_t)_MAC1) >> shift;
	MAC2 = (int32_t)(((int64_t)IR2 * IR0) + (int32_t)_MAC2) >> shift;
	MAC3 = (int32_t)(((int64_t)IR3 * IR0) + (int32_t)_MAC3) >> shift;
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();

	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX1) + (int64_t)((int16_t)L12 * (int16_t)VY1) + (int64_t)((int16_t)L13 * (int16_t)VZ1)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX1) + (int64_t)((int16_t)L22 * (int16_t)VY1) + (int64_t)((int16_t)L23 * (int16_t)VZ1)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX1) + (int64_t)((int16_t)L32 * (int16_t)VY1) + (int64_t)((int16_t)L33 * (int16_t)VZ1)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = ((int64_t)R * (int16_t)IR1) << 4;
	MAC2 = ((int64_t)G * (int16_t)IR2) << 4;
	MAC3 = ((int64_t)B * (int16_t)IR3) << 4;

	// Interpolate colour
	_MAC1 = MAC1;
	_MAC2 = MAC2;
	_MAC3 = MAC3;
	MAC1 = (int32_t)((((int64_t)RFC << 12) - (int32_t)MAC1) >> shift);
	MAC2 = (int32_t)((((int64_t)GFC << 12) - (int32_t)MAC2) >> shift);
	MAC3 = (int32_t)((((int64_t)BFC << 12) - (int32_t)MAC3) >> shift);
	IR1 = (int16_t)saturate(MAC1, -0x8000, 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000, 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000, 0x7fff);
	MAC1 = (int32_t)(((int64_t)IR1 * IR0) + (int32_t)_MAC1) >> shift;
	MAC2 = (int32_t)(((int64_t)IR2 * IR0) + (int32_t)_MAC2) >> shift;
	MAC3 = (int32_t)(((int64_t)IR3 * IR0) + (int32_t)_MAC3) >> shift;
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();

	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX2) + (int64_t)((int16_t)L12 * (int16_t)VY2) + (int64_t)((int16_t)L13 * (int16_t)VZ2)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX2) + (int64_t)((int16_t)L22 * (int16_t)VY2) + (int64_t)((int16_t)L23 * (int16_t)VZ2)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX2) + (int64_t)((int16_t)L32 * (int16_t)VY2) + (int64_t)((int16_t)L33 * (int16_t)VZ2)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = ((int64_t)R * (int16_t)IR1) << 4;
	MAC2 = ((int64_t)G * (int16_t)IR2) << 4;
	MAC3 = ((int64_t)B * (int16_t)IR3) << 4;

	// Interpolate colour
	_MAC1 = MAC1;
	_MAC2 = MAC2;
	_MAC3 = MAC3;
	MAC1 = (int32_t)((((int64_t)RFC << 12) - (int32_t)MAC1) >> shift);
	MAC2 = (int32_t)((((int64_t)GFC << 12) - (int32_t)MAC2) >> shift);
	MAC3 = (int32_t)((((int64_t)BFC << 12) - (int32_t)MAC3) >> shift);
	IR1 = (int16_t)saturate(MAC1, -0x8000, 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000, 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000, 0x7fff);
	MAC1 = (int32_t)(((int64_t)IR1 * IR0) + (int32_t)_MAC1) >> shift;
	MAC2 = (int32_t)(((int64_t)IR2 * IR0) + (int32_t)_MAC2) >> shift;
	MAC3 = (int32_t)(((int64_t)IR3 * IR0) + (int32_t)_MAC3) >> shift;
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandNCCS() {
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX0) + (int64_t)((int16_t)L12 * (int16_t)VY0) + (int64_t)((int16_t)L13 * (int16_t)VZ0)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX0) + (int64_t)((int16_t)L22 * (int16_t)VY0) + (int64_t)((int16_t)L23 * (int16_t)VZ0)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX0) + (int64_t)((int16_t)L32 * (int16_t)VY0) + (int64_t)((int16_t)L33 * (int16_t)VZ0)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = ((int64_t)R * (int16_t)IR1) << 4;
	MAC2 = ((int64_t)G * (int16_t)IR2) << 4;
	MAC3 = ((int64_t)B * (int16_t)IR3) << 4;

	MAC1 = (int32_t)MAC1 >> shift;
	MAC2 = (int32_t)MAC2 >> shift;
	MAC3 = (int32_t)MAC3 >> shift;

	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandNCS() {
	//printf("ncds\n");
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX0) + (int64_t)((int16_t)L12 * (int16_t)VY0) + (int64_t)((int16_t)L13 * (int16_t)VZ0)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX0) + (int64_t)((int16_t)L22 * (int16_t)VY0) + (int64_t)((int16_t)L23 * (int16_t)VZ0)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX0) + (int64_t)((int16_t)L32 * (int16_t)VY0) + (int64_t)((int16_t)L33 * (int16_t)VZ0)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandNCT() {
	//printf("ncds\n");
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX0) + (int64_t)((int16_t)L12 * (int16_t)VY0) + (int64_t)((int16_t)L13 * (int16_t)VZ0)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX0) + (int64_t)((int16_t)L22 * (int16_t)VY0) + (int64_t)((int16_t)L23 * (int16_t)VZ0)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX0) + (int64_t)((int16_t)L32 * (int16_t)VY0) + (int64_t)((int16_t)L33 * (int16_t)VZ0)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();

	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX1) + (int64_t)((int16_t)L12 * (int16_t)VY1) + (int64_t)((int16_t)L13 * (int16_t)VZ1)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX1) + (int64_t)((int16_t)L22 * (int16_t)VY1) + (int64_t)((int16_t)L23 * (int16_t)VZ1)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX1) + (int64_t)((int16_t)L32 * (int16_t)VY1) + (int64_t)((int16_t)L33 * (int16_t)VZ1)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();

	MAC1 = int32_t((int64_t)((int16_t)L11 * (int16_t)VX2) + (int64_t)((int16_t)L12 * (int16_t)VY2) + (int64_t)((int16_t)L13 * (int16_t)VZ2)) >> shift;
	MAC2 = int32_t((int64_t)((int16_t)L21 * (int16_t)VX2) + (int64_t)((int16_t)L22 * (int16_t)VY2) + (int64_t)((int16_t)L23 * (int16_t)VZ2)) >> shift;
	MAC3 = int32_t((int64_t)((int16_t)L31 * (int16_t)VX2) + (int64_t)((int16_t)L32 * (int16_t)VY2) + (int64_t)((int16_t)L33 * (int16_t)VZ2)) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	MAC1 = int32_t(((int64_t)RBK * 0x1000) + ((int64_t)((int16_t)LR1 * (int16_t)IR1) + (int64_t)((int16_t)LR2 * (int16_t)IR2) + (int64_t)((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t(((int64_t)GBK * 0x1000) + ((int64_t)((int16_t)LG1 * (int16_t)IR1) + (int64_t)((int16_t)LG2 * (int16_t)IR2) + (int64_t)((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC3 = int32_t(((int64_t)BBK * 0x1000) + ((int64_t)((int16_t)LB1 * (int16_t)IR1) + (int64_t)((int16_t)LB2 * (int16_t)IR2) + (int64_t)((int16_t)LB3 * (int16_t)IR3))) >> shift;
	//setIRFromMAC();
	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
	pushColour();
}

void gte::commandSQR() {
	const int shift = sf(instruction) * 12;
	MAC1 = (IR1 * IR1) >> shift;
	MAC2 = (IR2 * IR2) >> shift;
	MAC3 = (IR3 * IR3) >> shift;

	IR1 = saturate(MAC1, 0, 0x7fff);
	IR2 = saturate(MAC2, 0, 0x7fff);
	IR3 = saturate(MAC3, 0, 0x7fff);
}

void gte::commandNCLIP() {
	//printf("nclip\n");
	MAC0 = (int64_t)((int32_t)(SX0) * (int32_t)(SY1)) + ((int32_t)(SX1) * (int32_t)(SY2)) + ((int32_t)(SX2) * (int32_t)(SY0)) - ((int32_t)(SX0) * (int32_t)(SY2)) - ((int32_t)(SX1) * (int32_t)(SY0)) - ((int32_t)(SX2) * (int32_t)(SY1));
	//MAC0 = ((int64_t)(SX0 * SY1) + (SX1 * SY2) + (SX2 * SY0) - (SX0 * SY2) - (SX1 * SY0) - (SX2 * SY1));
	//printf("sx0: %d, sy0: %d sx1: %d, sy1: %d sx2: %d, sy2: %d\n", SX0, SY0, SX1, SY1, SX2, SY2);
	//auto a = (int32_t)(SX0) * ((int32_t)(SY1) - (int32_t)(SY2));
	//auto b = (int32_t)(SX1) * ((int32_t)(SY2) - (int32_t)(SY0));
	//auto c = (int32_t)(SX2) * ((int32_t)(SY0) - (int32_t)(SY1));
	//MAC0 = (int32_t)(a + b + c);
	//MAC0 = 0x1500000;
	//printf("%d\n", MAC0);
}

void gte::commandOP() {
	const int shift = sf(instruction) * 12;
	const int lm = this->lm(instruction);
	MAC1 = (int16_t)(((int16_t)IR3 * (int16_t)RT22) - ((int16_t)IR2 * (int16_t)RT33)) >> shift;
	MAC2 = (int16_t)(((int16_t)IR1 * (int16_t)RT33) - ((int16_t)IR3 * (int16_t)RT11)) >> shift;
	MAC3 = (int16_t)(((int16_t)IR2 * (int16_t)RT11) - ((int16_t)IR1 * (int16_t)RT22)) >> shift;

	IR1 = (int16_t)saturate(MAC1, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR2 = (int16_t)saturate(MAC2, -0x8000 * (lm ? 0 : 1), 0x7fff);
	IR3 = (int16_t)saturate(MAC3, -0x8000 * (lm ? 0 : 1), 0x7fff);
}

void gte::commandAVSZ3() {
	//printf("avsz3\n");
	MAC0 = (int64_t)ZSF3 * ((uint16_t)SZ1 + (uint16_t)SZ2 + (uint16_t)SZ3);
	//MAC0 = 0x10000;
	//MAC0 = ((int64_t)(ZSF3 * SZ1) + (ZSF3 * SZ2) + (ZSF3 * SZ3));
	OTZ = saturate((int32_t)MAC0 / 0x1000, 0, 0xffff);
}

void gte::commandAVSZ4() {
	//printf("avsz4\n");
	MAC0 = (int64_t)ZSF4 * ((uint16_t)SZ0 + (uint16_t)SZ1 + (uint16_t)SZ2 + (uint16_t)SZ3);
	OTZ = saturate((int32_t)MAC0 / 0x1000, 0, 0xffff);
}

void gte::commandRTPT() {
	//printf("rtpt\n");
	const int shift = sf(instruction) * 12;
	const int lm = 0;
	MAC1 = int64_t(((int64_t)(int32_t)(TRX) * 0x1000) + ((int16_t)RT11 * (int16_t)VX0) + ((int16_t)RT12 * (int16_t)VY0) + ((int16_t)RT13 * (int16_t)VZ0)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)(TRY) * 0x1000) + ((int16_t)RT21 * (int16_t)VX0) + ((int16_t)RT22 * (int16_t)VY0) + ((int16_t)RT23 * (int16_t)VZ0)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)(TRZ) * 0x1000) + ((int16_t)RT31 * (int16_t)VX0) + ((int16_t)RT32 * (int16_t)VY0) + ((int16_t)RT33 * (int16_t)VZ0)) >> shift;
	setIRFromMAC(lm);
	auto newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//uint32_t _proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	//int32_t _proj_factor = ((H * 0x20000) / (uint16_t)SZ3);
	int32_t _proj_factor = gte_divide(H, SZ3);
	int64_t proj_factor = (int64_t)(_proj_factor);
	int64_t _x = (int64_t)(int16_t)(IR1);
	int64_t _y = (int64_t)(int16_t)(IR2);
	int64_t x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	int64_t y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	x = saturate((x >> 16), -0x400, 0x3ff);
	y = saturate((y >> 16), -0x400, 0x3ff);
	SETSX2(x);
	SETSY2(y);
	SXY2 = (y << 16) | x;
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	/*int64_t depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = (int16_t)(depth);*/


	MAC1 = int64_t(((int64_t)(int32_t)(TRX) * 0x1000) + ((int16_t)RT11 * (int16_t)VX1) + ((int16_t)RT12 * (int16_t)VY1) + ((int16_t)RT13 * (int16_t)VZ1)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)(TRY) * 0x1000) + ((int16_t)RT21 * (int16_t)VX1) + ((int16_t)RT22 * (int16_t)VY1) + ((int16_t)RT23 * (int16_t)VZ1)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)(TRZ) * 0x1000) + ((int16_t)RT31 * (int16_t)VX1) + ((int16_t)RT32 * (int16_t)VY1) + ((int16_t)RT33 * (int16_t)VZ1)) >> shift;
	setIRFromMAC(lm);
	newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//_proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	//_proj_factor = ((H * 0x20000) / (uint16_t)SZ3);
	_proj_factor = gte_divide(H, SZ3);
	proj_factor = (int64_t)(_proj_factor);
	_x = (int64_t)(int16_t)(IR1);
	_y = (int64_t)(int16_t)(IR2);
	x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	x = saturate((x >> 16), -0x400, 0x3ff);
	y = saturate((y >> 16), -0x400, 0x3ff);
	SETSX2(x);
	SETSY2(y);
	SXY2 = (y << 16) | x;
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	/*depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = (int16_t)(depth);*/

	MAC1 = int64_t(((int64_t)(int32_t)(TRX) * 0x1000) + ((int16_t)RT11 * (int16_t)VX2) + ((int16_t)RT12 * (int16_t)VY2) + ((int16_t)RT13 * (int16_t)VZ2)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)(TRY) * 0x1000) + ((int16_t)RT21 * (int16_t)VX2) + ((int16_t)RT22 * (int16_t)VY2) + ((int16_t)RT23 * (int16_t)VZ2)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)(TRZ) * 0x1000) + ((int16_t)RT31 * (int16_t)VX2) + ((int16_t)RT32 * (int16_t)VY2) + ((int16_t)RT33 * (int16_t)VZ2)) >> shift;
	setIRFromMAC(lm);
	newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//_proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	//_proj_factor = ((H * 0x20000) / (uint16_t)SZ3);
	_proj_factor = gte_divide(H, SZ3);
	proj_factor = (int64_t)(_proj_factor);
	_x = (int64_t)(int16_t)(IR1);
	_y = (int64_t)(int16_t)(IR2);
	x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	x = saturate((x >> 16), -0x400, 0x3ff);
	y = saturate((y >> 16), -0x400, 0x3ff);
	SETSX2(x);
	SETSY2(y);
	SXY2 = (y << 16) | x;
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	int64_t depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = saturate((int16_t)(depth), 0, 0x1000);
}

void gte::commandGPF() {
	const int shift = sf(instruction) * 12;
	MAC1 = 0;
	MAC2 = 0;
	MAC3 = 0;

	MAC1 = (int32_t)((IR1 * IR0) + MAC1) >> shift;
	MAC2 = (int32_t)((IR2 * IR0) + MAC2) >> shift;
	MAC3 = (int32_t)((IR3 * IR0) + MAC3) >> shift;
	pushColour();
}

void gte::commandGPL() {
	const int shift = sf(instruction) * 12;
	MAC1 <<= shift;
	MAC2 <<= shift;
	MAC3 <<= shift;

	MAC1 = (int32_t)((IR1 * IR0) + MAC1) >> shift;
	MAC2 = (int32_t)((IR2 * IR0) + MAC2) >> shift;
	MAC3 = (int32_t)((IR3 * IR0) + MAC3) >> shift;
	pushColour();
}