#pragma once
#include <stdint.h>
#include <cstdarg>
#include <windows.h>
#include <array>
#include <vector>
#include "cdrom.h"
#include <fstream>
#include "logwindow.h"
#include "pad.h"
#include "gpu.h"

class memory
{
	// Our memory implementation uses software fastmem
	// Splitting the address space into 64KiB (2^16) pages, so 0x10000 pages
	// Here are our page tables
	std::vector<uintptr_t> readTable;
	std::vector<uintptr_t> writeTable;

public:
	memory();
	~memory();

	gpu* Gpu;

	uint32_t button_dest = 0;

	Log* logwnd;

	cdrom CDROM = cdrom();
	pad pads;
	std::vector<uint8_t> file;
	std::vector <uint8_t> bios;
	uint32_t adler32bios = 0;

	uint32_t* pc;
	uint32_t* regs;
	bool* shouldCheckDMA;
	uint8_t* ram = new uint8_t[0x200000];
	uint8_t* spu_ram = new uint8_t[0x80000];
	uint8_t* scratchpad = new uint8_t[1024];
	uint8_t* exp1 = new uint8_t[1024000];
	uint8_t* exp2 = new uint8_t[8000];
	//uint8_t* regs = new uint8_t[0xffffff];
	//uint8_t* mem = new uint8_t[0xffffffff];
	uint32_t RAM_SIZE = 0;
	uint32_t I_MASK = 0;
	uint32_t I_STAT = 0;
	uint32_t CACHE_CONTROL = 0;

	uint32_t exp2_delay_size = 0;

	uint32_t SPUSTAT = 0;

	// dma
	uint32_t DCPR = 0;
	uint32_t DICR = 0;

	// timer
	uint16_t tmr0_stub = 0;
	uint16_t tmr1_stub = 0;
	uint16_t tmr2_stub = 0;

	// spu
	uint16_t spu_ram_transfer_address = 0;


	typedef struct DMA {
		uint32_t MADR;
		uint32_t BCR;
		uint32_t CHCR;
	};

	DMA Ch2, Ch3, Ch4, Ch6;

	typedef struct Timer {
		uint16_t current_value;
		uint16_t counter_mode;
		uint16_t target_value;
	};

	Timer tmr0, tmr1, tmr2;


	uint32_t gpuread = 0;
	bool debug;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	void debug_log(const char* fmt, ...);
	void debug_warn(const char* fmt, ...);
	void debug_err(const char* fmt, ...);
	void loadBios(std::string directory);
	uint32_t loadExec(std::string directory);
	void write(uint32_t addr, uint8_t data, bool log);
	void write16(uint32_t addr, uint16_t data);
	void write32(uint32_t addr, uint32_t data);
	uint8_t read(uint32_t addr);
	uint16_t read16(uint32_t addr);
	uint32_t read32(uint32_t addr);

	uint32_t mask_address(const uint32_t addr);

	// HLE Syscalls
	void read_card_sector(int port, uint32_t sector, uint32_t dst);
};