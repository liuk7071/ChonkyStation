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
	case NCDS: cop2c[31] = 0; commandNCDS(); break;
	case AVSZ3: cop2c[31] = 0; commandAVSZ3(); break;
	case AVSZ4: cop2c[31] = 0; commandAVSZ4(); break;
	case RTPT: cop2c[31] = 0; commandRTPT(); break;
	default:
		printf("Unimplemented GTE instruction: 0x%x\n", instr);
		//exit(1);
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
		break;

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

	const uint32_t col = (((MAC1) / 16) << 0) | (((MAC2) / 16) << 8) | (((MAC3) / 16) << 16) | (CD2 << 24);
	RGB2 = col;
}

void gte::setIRFromMAC() {
	IR1 = MAC1 & 0xffff;
	IR2 = MAC2 & 0xffff;
	IR3 = MAC3 & 0xffff;
}

// Commands
void gte::moveMFC2(uint32_t* gpr) {
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
	//printf("cnt%d (0x%x) -> r%d\n", (instruction >> 11) & 0x1f, cop2c[(instruction >> 11) & 0x1f], (instruction >> 16) & 0x1f);
	gpr[(instruction >> 16) & 0x1f] = cop2c[(instruction >> 11) & 0x1f];
}
void gte::moveCTC2(uint32_t* gpr) {
	writeCop2c((instruction >> 11) & 0x1f, gpr[(instruction >> 16) & 0x1f]);
}

void gte::commandRTPS() {
	const int shift = sf(instruction) * 12;
	MAC1 = int64_t(((int64_t)(int32_t)TRX * 0x1000) + ((int16_t)RT11 * (int16_t)VX0) + ((int16_t)RT12 * (int16_t)VY0) + ((int16_t)RT13 * (int16_t)VZ0)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)TRY * 0x1000) + ((int16_t)RT21 * (int16_t)VX0) + ((int16_t)RT22 * (int16_t)VY0) + ((int16_t)RT23 * (int16_t)VZ0)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)TRZ * 0x1000) + ((int16_t)RT31 * (int16_t)VX0) + ((int16_t)RT32 * (int16_t)VY0) + ((int16_t)RT33 * (int16_t)VZ0)) >> shift;
	setIRFromMAC();
	auto newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//uint32_t _proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	int32_t _proj_factor = ((H * 0x20000) / SZ3);
	int64_t proj_factor = (int64_t)(_proj_factor);
	int64_t _x = (int64_t)(int16_t)(IR1);
	int64_t _y = (int64_t)(int16_t)(IR2);
	int64_t x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	int64_t y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	SETSX2((x >> 16));
	SETSY2((y >> 16));
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	int64_t depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = (int16_t)(depth);
}

void gte::commandNCLIP() {
	MAC0 = ((int32_t)(SX0) * (int32_t)(SY1)) + ((int32_t)(SX1) * (int32_t)(SY2)) + ((int32_t)(SX2) * (int32_t)(SY0)) - ((int32_t)(SX0) * (int32_t)(SY2)) - ((int32_t)(SX1) * (int32_t)(SY0)) - ((int32_t)(SX2) * (int32_t)(SY1));
	//printf("sx0: %d, sy0: %d sx1: %d, sy1: %d sx2: %d, sy2: %d\n", SX0, SY0, SX1, SY1, SX2, SY2);
	//auto a = (int32_t)(SX0) * ((int32_t)(SY1) - (int32_t)(SY2));
	//auto b = (int32_t)(SX1) * ((int32_t)(SY2) - (int32_t)(SY0));
	//auto c = (int32_t)(SX2) * ((int32_t)(SY0) - (int32_t)(SY1));
	//MAC0 = (int32_t)(a + b + c);
	////MAC0 = 0x70;
	//printf("%d\n", MAC0);
}

void gte::commandNCDS() {
	const int shift = sf(instruction) * 12;
	MAC1 = int32_t(((int16_t)L11 * (int16_t)VX0) + ((int16_t)L12 * (int16_t)VY0) + ((int16_t)L13 * (int16_t)VZ0)) >> shift;
	MAC2 = int32_t(((int16_t)L21 * (int16_t)VX0) + ((int16_t)L22 * (int16_t)VY0) + ((int16_t)L23 * (int16_t)VZ0)) >> shift;
	MAC3 = int32_t(((int16_t)L31 * (int16_t)VX0) + ((int16_t)L32 * (int16_t)VY0) + ((int16_t)L33 * (int16_t)VZ0)) >> shift;
	setIRFromMAC();
	MAC1 = int32_t((RBK * 0x1000) + (((int16_t)LR1 * (int16_t)IR1) + ((int16_t)LR2 * (int16_t)IR2) + ((int16_t)LR3 * (int16_t)IR3))) >> shift;
	MAC2 = int32_t((GBK * 0x1000) + (((int16_t)LG1 * (int16_t)IR1) + ((int16_t)LG2 * (int16_t)IR2) + ((int16_t)LG3 * (int16_t)IR3))) >> shift;
	MAC1 = int32_t((BBK * 0x1000) + (((int16_t)LB1 * (int16_t)IR1) + ((int16_t)LB2 * (int16_t)IR2) + ((int16_t)LB3 * (int16_t)IR3))) >> shift;
	setIRFromMAC();
	MAC1 = (R * ((int16_t)IR1)) << 4;
	MAC2 = (G * ((int16_t)IR2)) << 4;
	MAC3 = (B * ((int16_t)IR3)) << 4;
	MAC1 = MAC1 + ((RFC - (int32_t)MAC1) * ((int16_t)IR0));
	MAC2 = MAC2 + ((GFC - (int32_t)MAC2) * ((int16_t)IR0));
	MAC3 = MAC3 + ((BFC - (int32_t)MAC3) * ((int16_t)IR0));
	MAC1 = int32_t(MAC1) >> shift;
	MAC2 = int32_t(MAC2) >> shift;
	MAC3 = int32_t(MAC3) >> shift;
	
	pushColour();
	setIRFromMAC();
}

void gte::commandAVSZ3() {
	MAC0 = ZSF3 * (SZ1 + SZ2 + SZ3);
	OTZ = saturate(MAC0 / 0x1000, 0, 0xffff);
}

void gte::commandAVSZ4() {
	MAC0 = ZSF4 * (SZ0 + SZ1 + SZ2 + SZ3);
	OTZ = saturate(MAC0 / 0x1000, 0, 0xffff);
}

void gte::commandRTPT() {
	const int shift = sf(instruction) * 12;
	MAC1 = int64_t(((int64_t)(int32_t)(TRX) * 0x1000) + ((int16_t)RT11 * (int16_t)VX0) + ((int16_t)RT12 * (int16_t)VY0) + ((int16_t)RT13 * (int16_t)VZ0)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)(TRY) * 0x1000) + ((int16_t)RT21 * (int16_t)VX0) + ((int16_t)RT22 * (int16_t)VY0) + ((int16_t)RT23 * (int16_t)VZ0)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)(TRZ) * 0x1000) + ((int16_t)RT31 * (int16_t)VX0) + ((int16_t)RT32 * (int16_t)VY0) + ((int16_t)RT33 * (int16_t)VZ0)) >> shift;
	setIRFromMAC();
	auto newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//uint32_t _proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	int32_t _proj_factor = ((H * 0x20000) / SZ3);
	int64_t proj_factor = (int64_t)(_proj_factor);
	int64_t _x = (int64_t)(int16_t)(IR1);
	int64_t _y = (int64_t)(int16_t)(IR2);
	int64_t x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	int64_t y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	SETSX2((uint32_t)(x >> 16));
	SETSY2((uint32_t)(y >> 16));
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	int64_t depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = (int16_t)(depth);


	MAC1 = int64_t(((int64_t)(int32_t)(TRX) * 0x1000) + ((int16_t)RT11 * (int16_t)VX1) + ((int16_t)RT12 * (int16_t)VY1) + ((int16_t)RT13 * (int16_t)VZ1)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)(TRY) * 0x1000) + ((int16_t)RT21 * (int16_t)VX1) + ((int16_t)RT22 * (int16_t)VY1) + ((int16_t)RT23 * (int16_t)VZ1)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)(TRZ) * 0x1000) + ((int16_t)RT31 * (int16_t)VX1) + ((int16_t)RT32 * (int16_t)VY1) + ((int16_t)RT33 * (int16_t)VZ1)) >> shift;
	setIRFromMAC();
	newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//_proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	_proj_factor = ((H * 0x20000) / SZ3);
	proj_factor = (int64_t)(_proj_factor);
	_x = (int64_t)(int16_t)(IR1);
	_y = (int64_t)(int16_t)(IR2);
	x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	SETSX2((uint32_t)(x >> 16));
	SETSY2((uint32_t)(y >> 16));
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = (int16_t)(depth);

	MAC1 = int64_t(((int64_t)(int32_t)(TRX) * 0x1000) + ((int16_t)RT11 * (int16_t)VX2) + ((int16_t)RT12 * (int16_t)VY2) + ((int16_t)RT13 * (int16_t)VZ2)) >> shift;
	MAC2 = int64_t(((int64_t)(int32_t)(TRY) * 0x1000) + ((int16_t)RT21 * (int16_t)VX2) + ((int16_t)RT22 * (int16_t)VY2) + ((int16_t)RT23 * (int16_t)VZ2)) >> shift;
	MAC3 = int64_t(((int64_t)(int32_t)(TRZ) * 0x1000) + ((int16_t)RT31 * (int16_t)VX2) + ((int16_t)RT32 * (int16_t)VY2) + ((int16_t)RT33 * (int16_t)VZ2)) >> shift;
	setIRFromMAC();
	newZ = int32_t(MAC3) >> ((1 - sf(instruction)) * 12);
	pushZ(newZ);

	SXY0 = SXY1;
	SXY1 = SXY2;
	//_proj_factor = (((((uint32_t)(H) * 0x20000) / (uint32_t)(SZ3)) + 1) / 2);
	_proj_factor = ((H * 0x20000) / SZ3);
	proj_factor = (int64_t)(_proj_factor);
	_x = (int64_t)(int16_t)(IR1);
	_y = (int64_t)(int16_t)(IR2);
	x = ((_x * proj_factor) + (int64_t)(int32_t)(OFX));
	y = ((_y * proj_factor) + (int64_t)(int32_t)(OFY));
	SETSX2((uint32_t)(x >> 16));
	SETSY2((uint32_t)(y >> 16));
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR1)) + OFX; SETSX2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((int64_t)(((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * (int64_t)(int16_t)(IR2)) + OFY; SETSY2(((int32_t)(MAC0)) / 0x10000);
	//MAC0 = ((((((uint16_t)(H) * 0x20000) / (uint16_t)(SZ3)) + 1) / 2) * DQA) + DQB; IR0 = 
	depth = ((int64_t)DQB + ((int64_t)DQA * proj_factor));
	MAC0 = (int32_t)(depth);
	depth >>= 12;
	IR0 = (int16_t)(depth);
	printf("%x: %d, %d\n", SXY2, SX2, SY2);
}