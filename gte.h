#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <bit>
#include <iterator>
#include <intrin.h>

#define TEST_GTE

#define VX0 (cop2d.r[0].sw.l)
#define VY0 (cop2d.r[0].sw.h)
#define VZ0 (cop2d.r[1].sw.l)
#define VX1 (cop2d.r[2].w.l)
#define VY1 (cop2d.r[2].w.h)
#define VZ1 (cop2d.r[3].w.l)
#define VX2 (cop2d.r[4].w.l)
#define VY2 (cop2d.r[4].w.h)
#define VZ2 (cop2d.r[5].w.l)

#define RGBC cop2d.raw[6]
#define R (cop2d.r[6].b.l)
#define G (cop2d.r[6].b.h)
#define B (cop2d.r[6].b.h2)
#define CD2 (cop2d.r[6].b.h3)

#define OTZ (cop2d.r[7].w.l)

#define IR0 (cop2d.r[8].sw.l)
#define IR1 (cop2d.r[9].sw.l)
#define IR2 (cop2d.r[10].sw.l)
#define IR3 (cop2d.r[11].sw.l)

#define SXY0 (cop2d.r[12].d)
#define SX0 (cop2d.r[12].sw.l)
#define SY0 (cop2d.r[12].sw.h)
#define SXY1 (cop2d.r[13].d)
#define SX1 (cop2d.r[13].sw.l)
#define SY1 (cop2d.r[13].sw.h)
#define SXY2 (cop2d.r[14].d)
#define SX2 (cop2d.r[14].sw.l)
#define SY2 (cop2d.r[14].sw.h)
#define SXYP (cop2d.r[15].d)
#define SXP (cop2d.r[15].sw.l)
#define SYP (cop2d.r[15].sw.h)

#define SZ0 (cop2d.r[16].w.l)
#define SZ1 (cop2d.r[17].w.l)
#define SZ2 (cop2d.r[18].w.l)
#define SZ3 (cop2d.r[19].w.l)

#define RGB0 (cop2d.r[20].d)
#define R0 (cop2d.r[20].b.l)
#define G0 (cop2d.r[20].b.h)
#define B0 (cop2d.r[20].b.h2)
#define RGB1 (cop2d.r[21].d)
#define RGB2 (cop2d.r[22].d)

#define MAC0 (cop2d.r[24].sd)
#define MAC1 (cop2d.r[25].sd)
#define MAC2 (cop2d.r[26].sd)
#define MAC3 (cop2d.r[27].sd)

#define LZCS (cop2d.r[30].d)
#define LZCR (cop2d.r[31].d)


#define RT11 (cop2c.r[0].sw.l)
#define RT12 (cop2c.r[0].sw.h)
#define RT13 (cop2c.r[1].sw.l)
#define RT21 (cop2c.r[1].sw.h)
#define RT22 (cop2c.r[2].sw.l)
#define RT23 (cop2c.r[2].sw.h)
#define RT31 (cop2c.r[3].sw.l)
#define RT32 (cop2c.r[3].sw.h)
#define RT33 (cop2c.r[4].sw.l)

#define TRX (cop2c.r[5].sd)
#define TRY (cop2c.r[6].sd)
#define TRZ (cop2c.r[7].sd)

#define L11 (cop2c.r[8].sw.l)
#define L12 (cop2c.r[8].sw.h)
#define L13 (cop2c.r[9].sw.l)
#define L21 (cop2c.r[9].sw.h)
#define L22 (cop2c.r[10].sw.l)
#define L23 (cop2c.r[10].sw.h)
#define L31 (cop2c.r[11].sw.l)
#define L32 (cop2c.r[11].sw.h)
#define L33 (cop2c.r[12].sw.l)

#define RBK (cop2c.r[13].sd)
#define GBK (cop2c.r[14].sd)
#define BBK (cop2c.r[15].sd)

#define LR1 (cop2c.r[16].sw.l)
#define LR2 (cop2c.r[16].sw.h)
#define LR3 (cop2c.r[17].sw.l)
#define LG1 (cop2c.r[17].sw.h)
#define LG2 (cop2c.r[18].sw.l)
#define LG3 (cop2c.r[18].sw.h)
#define LB1 (cop2c.r[19].sw.l)
#define LB2 (cop2c.r[19].sw.h)
#define LB3 (cop2c.r[20].sw.l)

#define RFC (cop2c.r[21].sd)
#define GFC (cop2c.r[22].sd)
#define BFC (cop2c.r[23].sd)

#define OFX (cop2c.r[24].sd)
#define OFY (cop2c.r[25].sd)

#define H (cop2c.r[26].sw.l)

#define DQA (cop2c.r[27].sw.l)
#define DQB (cop2c.r[28].sd)

#define ZSF3 (cop2c.r[29].sw.l)
#define ZSF4 (cop2c.r[30].sw.l)

class gte
{
public:
	gte() {

	}
	typedef union {
		struct {
			uint8_t l, h, h2, h3;
		} b;
		struct {
			uint16_t l, h;
		} w;
		struct {
			int8_t l, h, h2, h3;
		} sb;
		struct {
			int16_t l, h;
		} sw;
		uint32_t d;
		int32_t sd;
	} pair;
	union {
		struct {
			int16_t vx0, vy0, vz0;
			int16_t vx1, vy1, vz1;
			int16_t vx2, vy2, vz2;
			uint8_t r, g, b, c;
			int32_t otz;
			int32_t ir0, ir1, ir2, ir3;
			int16_t sx0, sy0;
			int16_t sx1, sy1;
			int16_t sx2, sy2;
			int16_t sz0, n0;
			int16_t sz1, n1;
			int16_t sz2, n2;
			uint8_t r0, g0, b0, c0;
			uint8_t r1, g1, b1, c1;
			uint8_t r2, g2, b2, c2;
			int32_t reserved;
			int32_t mac0, mac1, mac2, mac3;
			uint32_t irgb, orgb;
			int32_t lzcs, lzcr;
		} regs;
		uint32_t raw[32];
		pair r[32];
	} cop2d;

	union {
		struct {
			int16_t r11, r12, r13, r21, r22, r23, r31, r32, r33, n0;
			int32_t trX, trY, trZ;
			int16_t l11, l12, l13, l21, l22, l23, l31, l32, l33, n1;
			int32_t rbk, gbk, bbk;
			int16_t lr1, lr2, lr3, lg1, lg2, lg3, lb1, lb2, lb3, n2;
			int32_t rfc, gfc, bfc;
			int32_t ofx, ofy;
			int32_t h;
			int32_t dqa, dqb;
			int32_t zsf3, zsf4;
			int32_t flag;
		} regs;
		uint32_t raw[32];
		pair r[32];
	} cop2c;

	//uint32_t cop2d.raw[32];
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
	uint32_t m(uint32_t instr) { return ((instr >> 17) & 3); }
	uint32_t v(uint32_t instr) { return ((instr >> 15) & 3); }
	uint32_t cv(uint32_t instr) { return ((instr >> 13) & 3); }
	int16_t mx(int x, int i);
	int16_t vx(int x, int i);
	int64_t tx(int x, int i);
	uint32_t instruction = 0;
	enum Commands {
		MOVE,
		RTPS = 0x01,
		NCLIP = 0x06,
		OP = 0x0c,
		DPCS = 0x10,
		INTPL = 0x11,
		MVMVA = 0x12,
		NCDS = 0x13,
		NCDT = 0x16,
		NCCS = 0x1b,
		NCS = 0x1e,
		NCT = 0x20,
		SQR = 0x28,
		DPCT = 0x2a,
		AVSZ3 = 0x2d,
		AVSZ4 = 0x2e,
		RTPT = 0x30,
		GPF = 0x3d,
		GPL = 0x3e,
		NCCT = 0x3f
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
	void commandOP();
	void commandDPCS();
	void commandINTPL();
	void commandMVMVA();
	void commandNCDS();
	void commandNCDT();
	void commandNCCS();
	void commandNCS();
	void commandNCT();
	void commandSQR();
	void commandDPCT();
	void commandAVSZ3();
	void commandAVSZ4();
	void commandRTPT();
	void commandGPF();
	void commandGPL();
	void commandNCCT();
	
	// Helpers
	static uint32_t countLeadingZeros16(uint16_t value) {
		int count = __lzcnt(value);
		return count - 16;
	}
	static uint32_t gte_divide(uint16_t numerator, uint16_t denominator) {
		if (numerator >= denominator * 2) {  // Division overflow
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

