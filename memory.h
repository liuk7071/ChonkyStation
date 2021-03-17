#pragma once
#include <stdint.h>
#include <array>

class memory
{
public:
	memory();
	~memory();
public:
	uint8_t* ram = new uint8_t[0x200000];
	uint8_t* bios = new uint8_t[524288];
	uint8_t* exp1 = new uint8_t[0xffffff];
	uint8_t* exp2 = new uint8_t[0xffffff];
	//uint8_t* regs = new uint8_t[0xffffff];
	//uint8_t* mem = new uint8_t[0xffffffff];
	uint32_t RAM_SIZE;
	uint32_t IRQ_MASK;
	uint32_t IRQ_STATUS;
	uint32_t CACHE_CONTROL;

	uint32_t SPUSTAT;

	uint32_t DCPR;
	uint32_t DICR;

	uint32_t gp0;
	uint32_t gp1;
	uint32_t gpuread;
	uint32_t gpustat;
	
	
public:
	bool debug;
	bool inRange(unsigned low, unsigned high, unsigned x)
	{
		return (low <= x && x <= high);
	}
	void loadBios();
	uint32_t loadExec();
	void write(uint32_t addr, uint8_t data, bool log);
	void write16(uint32_t addr, uint16_t data);
	void write32(uint32_t addr, uint32_t data);
	uint8_t read(uint32_t addr);
	uint16_t read16(uint32_t addr);
	uint32_t read32(uint32_t addr);
	
	uint32_t mask_address(const uint32_t addr);
};
