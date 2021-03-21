#include <stdint.h>
#include <cstdarg>
#include <string.h>
#include "Bus.h"
#include "cop0.h"
class cpu
{
public:
	cpu();
	~cpu();
	void debug_printf(const char* fmt, ...);
	std::string reg[32] = { "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3","$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7","$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };
	
public:
	enum exceptions {
		SysCall = 0x8,
		Overflow = 0xC
	};
	cop0 COP0 = cop0();
	Bus bus = Bus();
	
public:
	uint32_t fetch(uint32_t addr);
	void execute(uint32_t instr);
public:
	void exception(exceptions);
	uint32_t jump; // jump branch delay slot
public:
	// registers
	uint32_t pc;
	uint32_t sp;
	uint32_t zero;
	uint32_t regs[32];
	uint32_t hi;
	uint32_t lo;

public:
	void do_dma(int channel);

public:
	bool debug;
	bool log_kernel;
	bool exe;
	bool tty;
};

