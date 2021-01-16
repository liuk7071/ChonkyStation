#pragma once
#include <stdint.h>
#include <array>

class memory
{
public:
	memory();
	~memory();
public:
	
	uint8_t* mem = new uint8_t[0xffffffff];
	
	
public:
	void loadBios();
	void write(uint32_t addr, uint8_t data);
	uint8_t read(uint32_t addr);
	uint32_t read32(uint32_t addr);
};
