#pragma once
#include <stdint.h>
#include <array>
#include <vector>
#include "cdrom.h"

class memory
{
public:
	memory();
	~memory();

	cdrom CDROM = cdrom();
	std::vector<uint8_t> file;

public:
	uint8_t* ram = new uint8_t[0x200000];
	uint8_t* scratchpad = new uint8_t[1024];
	uint8_t* bios = new uint8_t[524288];
	uint8_t* exp1 = new uint8_t[1024000];
	uint8_t* exp2 = new uint8_t[8000];
	//uint8_t* regs = new uint8_t[0xffffff];
	//uint8_t* mem = new uint8_t[0xffffffff];
	uint32_t RAM_SIZE;
	uint32_t IRQ_MASK;
	uint32_t IRQ_STATUS;
	uint32_t CACHE_CONTROL;

	uint32_t exp2_delay_size;

	uint32_t SPUSTAT;

	// dma
	uint32_t DCPR;
	uint32_t DICR;

	
	
	// channel 2
	uint32_t channel2_base_address;
	uint32_t channel2_block_control;
	uint32_t channel2_control;
	
	// channel 6
	uint32_t channel6_base_address;
	uint32_t channel6_block_control;
	uint32_t channel6_control;

	uint32_t gp0;
	uint32_t gp1;
	uint32_t gpuread;
	uint32_t gpustat;

	// cdrom
	uint8_t cdrom_status;
	uint8_t cdrom_request;
	uint8_t cdrom_interrupt_enable;
	uint8_t cdrom_interrupt_flag;

	
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
