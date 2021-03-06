#include <stdint.h>
#include <cstdarg>
#include <string.h>
#include <windows.h>
#include "Bus.h"
#include "cop0.h"

class cpu
{
public:
	cpu(std::string rom_directory, std::string bios_directory, bool running_in_ci);
	~cpu();
	void debug_log(const char* fmt, ...);
	void debug_warn(const char* fmt, ...);
	void debug_err(const char* fmt, ...);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::string reg[32] = { "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3","$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7","$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };
public:
	
public:
	enum exceptions {
		INT = 0x0,
		BadFetchAddr = 0x4,
		BadStoreAddr = 0x5,
		SysCall = 0x8,
		Break = 0x9,
		Reserved_Instruction = 0xA,
		Overflow = 0xC
	};

	cop0 COP0 = cop0();
	Bus bus = Bus();

	uint32_t next_instr = 0;
public:
	uint32_t fetch(uint32_t addr);
	void execute(uint32_t instr);
public:
	void exception(exceptions);
	uint32_t jump; // jump branch delay slot
public:
	// registers
	uint32_t pc;
	uint32_t regs[32];
	uint32_t hi;
	uint32_t lo;

public:
	void check_dma();
	void do_dma(int channel);
	void check_CDROM_IRQ();
	void step();
	int frame_cycles;
	void sideloadExecutable(std::string directory);

public:
	bool debug;
	bool log_kernel;
	bool exe;
	bool tty;

	bool delay;
	std::string rom_directory;
};
