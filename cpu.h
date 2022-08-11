#include <stdint.h>
#include <cstdarg>
#include <string.h>
#define NOMINMAX
#include <windows.h>
#include "Bus.h"
#include "cop0.h"
#include "gte.h"
#include "logwindow.h"

#define log_cpu
#define log_kernel_tty
#undef log_cpu

class cpu
{
public:
	cpu(std::string rom_directory, std::string bios_directory, bool running_in_ci);
	void reset();
	~cpu();
	Log log;
	void debug_warn(const char* fmt, ...);
	void debug_err(const char* fmt, ...);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::string reg[32] = { "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3","$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7","$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };

	bool patch_b0_12h = false;
	uint32_t button_dest = 0;

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
	gte GTE;
	Bus bus = Bus();

	uint32_t next_instr = 0;
	uint32_t jump = 0; // jump branch delay slot
	// registers
	uint32_t pc = 0;
	uint32_t regs[32];
	uint32_t hi = 0;
	uint32_t lo = 0;

	void execute(uint32_t instr);
	void exception(exceptions);
	void check_dma();

	bool should_service_dma_irq = false;
	template<int channel> void do_dma();
	void check_CDROM_IRQ();
	void step();
	int frame_cycles = 0;
	int read_delay = 33868800 / 75;
	void sideloadExecutable(std::string directory);

	bool debug = false;
	bool log_kernel = false;
	bool exe = false;
	bool tty = false;

	bool delay = false;
	bool shouldCheckDMA = false;
	std::string rom_directory;
};