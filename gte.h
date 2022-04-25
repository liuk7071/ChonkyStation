#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <bit>
#include <iterator>
#include <intrin.h>

#define TEST_GTE

#define VY0 ((int16_t)(cop2d[0] >> 16))
#define VX0 ((int16_t)(cop2d[0] & 0xffff))
#define VZ0 ((int16_t)cop2d[1])

#define VY1 ((int16_t)(cop2d[2] >> 16))
#define VX1 ((int16_t)(cop2d[2] & 0xffff))
#define VZ1 ((int16_t)(cop2d[3]))

#define VY2 ((int16_t)(cop2d[4] >> 16))
#define VX2 ((int16_t)(cop2d[4] & 0xffff))
#define VZ2 ((int16_t)cop2d[5])

#define RGBC cop2d[6]
#define R (cop2d[6] & 0xff)
#define G ((cop2d[6] >> 8) & 0xff)
#define B ((cop2d[6] >> 16) & 0xff)
#define CD2 ((cop2d[6] >> 24) & 0xff)

#define OTZ (cop2d[7])

#define IR0 cop2d[8]
#define IR1 cop2d[9]
#define IR2 cop2d[10]
#define IR3 cop2d[11]

#define SETSY0(value)	    \
cop2d[12] &= ~0xffff0000;   \
cop2d[12] |= ((value) << 16) 
#define SETSX0(value)	    \
cop2d[12] &= ~0xffff;       \
cop2d[12] |= ((uint16_t)(value)) 

#define SY0 ((int16_t)(cop2d[12] >> 16))
#define SX0 ((int16_t)(cop2d[12] & 0xffff))

#define SETSY1(value)	    \
cop2d[13] &= ~0xffff0000;   \
cop2d[13] |= ((value) << 16) 
#define SETSX1(value)	    \
cop2d[13] &= ~0xffff;       \
cop2d[13] |= ((uint16_t)(value)) 

#define SY1 ((int16_t)(cop2d[13] >> 16))
#define SX1 ((int16_t)(cop2d[13] & 0xffff))

#define SETSY2(value)	    \
cop2d[14] &= ~0xffff0000;   \
cop2d[14] |= ((value) << 16) 
#define SETSX2(value)	    \
cop2d[14] &= ~0xffff;       \
cop2d[14] |= ((uint16_t)(value))

#define SY2 ((int16_t)(cop2d[14] >> 16))
#define SX2 ((int16_t)(cop2d[14] & 0xffff))

#define SYP ((int16_t)(cop2d[15] >> 16))
#define SXP ((int16_t)(cop2d[15] & 0xffff))

#define SXY0 (cop2d[12])
#define SXY1 (cop2d[13])
#define SXY2 (cop2d[14])

#define SZ0 cop2d[16]
#define SZ1 cop2d[17]
#define SZ2 cop2d[18]
#define SZ3 cop2d[19]

#define RGB0 cop2d[20]
#define RGB1 cop2d[21]
#define RGB2 cop2d[22]

#define MAC0 cop2d[24]
#define MAC1 cop2d[25]
#define MAC2 cop2d[26]
#define MAC3 cop2d[27]


#define RT12 ((int16_t)(cop2c[0] >> 16))
#define RT11 ((int16_t)(cop2c[0] & 0xffff))
#define RT21 ((int16_t)(cop2c[1] >> 16))
#define RT13 ((int16_t)(cop2c[1] & 0xffff))
#define RT23 ((int16_t)(cop2c[2] >> 16))
#define RT22 ((int16_t)(cop2c[2] & 0xffff))
#define RT32 ((int16_t)(cop2c[3] >> 16))
#define RT31 ((int16_t)(cop2c[3] & 0xffff))
#define RT33 ((int16_t)(cop2c[4] & 0xffff))

#define TRX cop2c[5]
#define TRY cop2c[6]
#define TRZ cop2c[7]

#define L12 ((int16_t)(cop2c[8] >> 16))
#define L11 ((int16_t)(cop2c[8] & 0xffff))
#define L21 ((int16_t)(cop2c[9] >> 16))
#define L13 ((int16_t)(cop2c[9] & 0xffff))
#define L23 ((int16_t)(cop2c[10] >> 16))
#define L22 ((int16_t)(cop2c[10] & 0xffff))
#define L32 ((int16_t)(cop2c[11] >> 16))
#define L31 ((int16_t)(cop2c[11] & 0xffff))
#define L33 ((int16_t)(cop2c[12] & 0xffff))

#define RBK cop2c[13]
#define GBK cop2c[14]
#define BBK cop2c[15]

#define LR2 ((int16_t)(cop2c[16] >> 16))
#define LR1 ((int16_t)(cop2c[16] & 0xffff))
#define LG1 ((int16_t)(cop2c[17] >> 16))
#define LR3 ((int16_t)(cop2c[17] & 0xffff))
#define LG3 ((int16_t)(cop2c[18] >> 16))
#define LG2 ((int16_t)(cop2c[18] & 0xffff))
#define LB2 ((int16_t)(cop2c[19] >> 16))
#define LB1 ((int16_t)(cop2c[19] & 0xffff))
#define LB3 ((int16_t)(cop2c[20] & 0xffff))

#define RFC cop2c[21]
#define GFC cop2c[22]
#define BFC cop2c[23]

#define OFX cop2c[24]
#define OFY cop2c[25]

#define H ((uint16_t)cop2c[26])

#define DQA ((int16_t)cop2c[27])
#define DQB cop2c[28]

#define ZSF3 (int16_t)cop2c[29]
#define ZSF4 (int16_t)cop2c[30]

class gte
{
public:
	gte() {

	}

	uint32_t cop2c[32];
	uint32_t cop2d[32];
	std::string cop2cNames[32] = {
	"r11r12", "r13r21", "r22r23", "r31r32", "r33", "trx",  "try",  "trz",   // 00
	"l11l12", "l13l21", "l22l23", "l31l32", "l33", "rbk",  "gbk",  "bbk",   // 08
	"lr1lr2", "lr3lg1", "lg2lg3", "lb1lb2", "lb3", "rfc",  "gfc",  "bfc",   // 10
	"ofx",    "ofy",    "h",      "dqa",    "dqb", "zsf3", "zsf4", "flag",  // 18
	};
	std::string cop2dNames[32] = {
	"vxy0", "vz0",  "vxy1", "vz1",  "vxy2", "vz2",  "rgb",  "otz",   // 00
	"ir0",  "ir1",  "ir2",  "ir3",  "sxy0", "sxy1", "sxy2", "sxyp",  // 08
	"sz0",  "sz1",  "sz2",  "sz3",  "rgb0", "rgb1", "rgb2", "res1",  // 10
	"mac0", "mac1", "mac2", "mac3", "irgb", "orgb", "lzcs", "lzcr",  // 18
	};
	void execute(uint32_t instr, uint32_t* gpr);
	uint32_t sf(uint32_t instr) { return ((instr >> 19) & 1); }
	uint32_t lm(uint32_t instr) { return ((instr >> 10) & 1); }
	uint32_t instruction = 0;
	enum Commands {
		MOVE,
		RTPS = 0x01,
		NCLIP = 0x06,
		NCDS = 0x13,
		AVSZ3 = 0x2d,
		AVSZ4 = 0x2e,
		RTPT = 0x30
	};
	enum Move {
		MFC2 = 0,
		CFC2 = 2,
		MTC2 = 4,
		CTC2 = 6
	};

	// Commands
	void moveMFC2(uint32_t* gpr);
	void moveMTC2(uint32_t* gpr);
	void moveCFC2(uint32_t* gpr);
	void moveCTC2(uint32_t* gpr);
	void commandRTPS();
	void commandNCLIP();
	void commandNCDS();
	void commandAVSZ3();
	void commandAVSZ4();
	void commandRTPT();
	
	// Helpers
	static uint32_t countLeadingZeros16(uint16_t value) {
		// Use a 32-bit CLZ as it's what's most commonly available and Clang/GCC fail to optimize 16-bit CLZ
		int count = __lzcnt(value);
		return count - 16;
	}
	static uint32_t gte_divide(uint16_t numerator, uint16_t denominator) {
		if (numerator >= denominator * 2) {  // Division overflow
			//FLAG |= (1 << 31) | (1 << 17);
			return 0x1ffff;
		}

		static uint8_t table[] = {
			0xff, 0xfd, 0xfb, 0xf9, 0xf7, 0xf5, 0xf3, 0xf1, 0xef, 0xee, 0xec, 0xea, 0xe8, 0xe6, 0xe4, 0xe3, 0xe1, 0xdf,
			0xdd, 0xdc, 0xda, 0xd8, 0xd6, 0xd5, 0xd3, 0xd1, 0xd0, 0xce, 0xcd, 0xcb, 0xc9, 0xc8, 0xc6, 0xc5, 0xc3, 0xc1,
			0xc0, 0xbe, 0xbd, 0xbb, 0xba, 0xb8, 0xb7, 0xb5, 0xb4, 0xb2, 0xb1, 0xb0, 0xae, 0xad, 0xab, 0xaa, 0xa9, 0xa7,
			0xa6, 0xa4, 0xa3, 0xa2, 0xa0, 0x9f, 0x9e, 0x9c, 0x9b, 0x9a, 0x99, 0x97, 0x96, 0x95, 0x94, 0x92, 0x91, 0x90,
			0x8f, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b,
			0x7a, 0x79, 0x78, 0x77, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70, 0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68,
			0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60, 0x5f, 0x5e, 0x5d, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58, 0x57,
			0x56, 0x55, 0x54, 0x53, 0x53, 0x52, 0x51, 0x50, 0x4f, 0x4e, 0x4d, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x48,
			0x47, 0x46, 0x45, 0x44, 0x43, 0x43, 0x42, 0x41, 0x40, 0x3f, 0x3f, 0x3e, 0x3d, 0x3c, 0x3c, 0x3b, 0x3a, 0x39,
			0x39, 0x38, 0x37, 0x36, 0x36, 0x35, 0x34, 0x33, 0x33, 0x32, 0x31, 0x31, 0x30, 0x2f, 0x2e, 0x2e, 0x2d, 0x2c,
			0x2c, 0x2b, 0x2a, 0x2a, 0x29, 0x28, 0x28, 0x27, 0x26, 0x26, 0x25, 0x24, 0x24, 0x23, 0x22, 0x22, 0x21, 0x20,
			0x20, 0x1f, 0x1e, 0x1e, 0x1d, 0x1d, 0x1c, 0x1b, 0x1b, 0x1a, 0x19, 0x19, 0x18, 0x18, 0x17, 0x16, 0x16, 0x15,
			0x15, 0x14, 0x14, 0x13, 0x12, 0x12, 0x11, 0x11, 0x10, 0x0f, 0x0f, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c, 0x0b,
			0x0a, 0x0a, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02,
			0x01, 0x01, 0x00, 0x00, 0x00 };

		int shift = countLeadingZeros16(denominator);

		int r1 = (denominator << shift) & 0x7fff;
		int r2 = table[((r1 + 0x40) >> 7)] + 0x101;
		int r3 = ((0x80 - (r2 * (r1 + 0x8000))) >> 8) & 0x1ffff;
		uint32_t reciprocal = ((r2 * r3) + 0x80) >> 8;

		const uint32_t res = ((((uint64_t)reciprocal * (numerator << shift)) + 0x8000) >> 16);

		// Some divisions like 0xF015/0x780B result in 0x20000, but are saturated to 0x1ffff without setting FLAG
		return std::min<uint32_t>(0x1ffff, res);
	}
	uint32_t readCop2d(uint32_t reg);
	void writeCop2d(uint32_t reg, uint32_t val);
	void writeCop2c(uint32_t reg, uint32_t val);
	void pushZ(uint16_t value);
	void pushColour();
	void setIRFromMAC(bool lm);
	
	static int32_t saturate(int32_t val, int32_t min, int32_t max) {
		if (val > max) {
			return max;
		}
		else if (val < min) {
			return min;
		}
		else {
			return val;
		}
	}
};

