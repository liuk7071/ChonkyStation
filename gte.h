#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>

#define VX0 (cop2d[0] >> 16)
#define VY0 (cop2d[0] & 0xffff)
#define VZ0 cop2d[1]

#define VX1 (cop2d[2] >> 16)
#define VY1 (cop2d[2] & 0xffff)
#define VZ1 cop2d[3]

#define VX2 (cop2d[4] >> 16)
#define VY2 (cop2d[4] & 0xffff)
#define VZ2 cop2d[5]

#define RGBC cop2d[6]
#define R (cop2d[6] & 0xff)
#define G ((cop2d[6] >> 8) & 0xff)
#define B ((cop2d[6] >> 16) & 0xff)
#define CD2 ((cop2d[6] >> 24) & 0xff)

#define OTZ cop2d[7]

#define IR0 cop2d[8]
#define IR1 cop2d[9]
#define IR2 cop2d[10]
#define IR3 cop2d[11]

#define SETSY0(value)	    \
cop2d[12] &= ~0xffff0000;   \
cop2d[12] |= ((value) << 16) 
#define SETSX0(value)	    \
cop2d[12] &= ~0xffff;       \
cop2d[12] |= (value) 

#define SY0 (cop2d[12] >> 16)
#define SX0 (cop2d[12] & 0xffff)

#define SETSY1(value)	    \
cop2d[13] &= ~0xffff0000;   \
cop2d[13] |= ((value) << 16) 
#define SETSX1(value)	    \
cop2d[13] &= ~0xffff;       \
cop2d[13] |= (value) 

#define SY1 (cop2d[13] >> 16)
#define SX1 (cop2d[13] & 0xffff)

#define SETSY2(value)	    \
cop2d[14] &= ~0xffff0000;   \
cop2d[14] |= ((value) << 16) 
#define SETSX2(value)	    \
cop2d[14] &= ~0xffff;       \
cop2d[14] |= (value) 

#define SY2 (cop2d[14] >> 16)
#define SX2 (cop2d[14] & 0xffff)

#define SYP (cop2d[15] >> 16)
#define SXP (cop2d[15] & 0xffff)

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


#define RT12 (cop2c[0] >> 16)
#define RT11 (cop2c[0] & 0xffff)
#define RT21 (cop2c[1] >> 16)
#define RT13 (cop2c[1] & 0xffff)
#define RT23 (cop2c[2] >> 16)
#define RT22 (cop2c[2] & 0xffff)
#define RT32 (cop2c[3] >> 16)
#define RT31 (cop2c[3] & 0xffff)
#define RT33 (cop2c[4] & 0xffff)

#define TRX cop2c[5]
#define TRY cop2c[6]
#define TRZ cop2c[7]

#define L12 (cop2c[8] >> 16)
#define L11 (cop2c[8] & 0xffff)
#define L21 (cop2c[9] >> 16)
#define L13 (cop2c[9] & 0xffff)
#define L23 (cop2c[10] >> 16)
#define L22 (cop2c[10] & 0xffff)
#define L32 (cop2c[11] >> 16)
#define L31 (cop2c[11] & 0xffff)
#define L33 (cop2c[12] & 0xffff)

#define RBK cop2c[13]
#define GBK cop2c[14]
#define BBK cop2c[15]

#define LR1 (cop2c[16] & 0xffff)
#define LR2 (cop2c[16] >> 16)
#define LR3 (cop2c[17] & 0xffff)
#define LG1 (cop2c[17] >> 16)
#define LG2 (cop2c[18] & 0xffff)
#define LG3 (cop2c[18] >> 16)
#define LB1 (cop2c[19] & 0xffff)
#define LB2 (cop2c[19] >> 16)
#define LB3 (cop2c[16] & 0xffff)

#define RFC cop2c[21]
#define GFC cop2c[22]
#define BFC cop2c[23]

#define OFX cop2c[24]
#define OFY cop2c[25]

#define H (cop2c[26])

#define DQA cop2c[27]
#define DQB cop2c[28]

#define ZSF3 cop2c[29]
#define ZSF4 cop2c[30]


#define sf(instr) ((instr >> 19) & 1)

class gte
{
public:
	uint32_t cop2c[32];
	uint32_t cop2d[32];
	void execute(uint32_t instr, uint32_t* gpr);
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
};

