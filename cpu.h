#include <stdint.h>
#include <cstdarg>
#include "Bus.h"
#include "cop0.h"
class cpu
{
public:
	cpu();
	~cpu();
	void debug_printf(const char* fmt, ...);
	
public:
	enum exceptions {
		SysCall = 0x8
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
	bool debug;
	bool exe;
};

