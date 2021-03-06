#pragma once
#include <stdint.h>
#include <cstdarg>
#include <windows.h>
#include <array>
#include <vector>
#include "cdrom.h"
#include <fstream>
#include "controller.h"

class memory
{
public:
	memory();
	~memory();

	cdrom CDROM = cdrom();
	controller pad1 = controller();
	std::vector<uint8_t> file;
	std::vector <uint8_t> bios;

public:
	uint8_t* ram = new uint8_t[0x200000];
	uint8_t* scratchpad = new uint8_t[1024];
	uint8_t* exp1 = new uint8_t[1024000];
	uint8_t* exp2 = new uint8_t[8000];
	//uint8_t* regs = new uint8_t[0xffffff];
	//uint8_t* mem = new uint8_t[0xffffffff];
	uint32_t RAM_SIZE;
	uint32_t I_MASK;
	uint32_t I_STAT;
	uint32_t CACHE_CONTROL;

	uint32_t exp2_delay_size;

	uint32_t SPUSTAT;

	// dma
	uint32_t DCPR;
	uint32_t DICR;


	typedef struct DMA {
		uint32_t MADR;
		uint32_t BCR;
		uint32_t CHCR;
	};

	DMA Ch2, Ch3, Ch6;


	uint32_t gpuread;
public:
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
};