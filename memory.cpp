#include "memory.h"
#include <fstream>
#pragma warning(disable : 4996)
memory::memory() {
	debug = true;
}

memory::~memory() {

}


uint32_t memory::mask_address(const uint32_t addr)
{
	static const uint32_t ADDR_MASK[] =
	{
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x7FFFFFFF,													// totally my code
		0x1FFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF
	};

	const auto idx = (addr >> 29u);

	return addr & ADDR_MASK[idx];
}

uint8_t memory::read(uint32_t addr) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);
	

	if (addr == 0x1F801074) exit(0);

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 0x7D000) {
		memcpy(&bytes, &bios[masked_addr & 0xfffff], sizeof(uint8_t));
		return bytes;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0xfffff], sizeof(uint8_t));
		return bytes;
	}
	//if (addr >= 0x00200000 && addr < 0x00200000 + 0x200000 || addr >= 0x80200000 && addr < 0x80200000 + 0x200000 || addr >= 0xA0200000 && addr < 0xA0200000 + 0x200000) {
	//	memcpy(&bytes, &ram[(masked_addr & 0xfffff) - 0x200000], sizeof(uint8_t));
	//	return bytes;
	//}
	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {	// return default exp1 value
		return 0xff;
	}	

	printf("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

uint16_t memory::read16(uint32_t addr) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);

	if (addr == 0x1F801074) exit(0);

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 0x7D000) {
		memcpy(&bytes, &bios[masked_addr & 0xfffff], sizeof(uint16_t));
		return bytes;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0xfffff], sizeof(uint16_t));
		return bytes;
	}
	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {
		return 0xffff;
	}
	

	printf("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

uint32_t memory::read32(uint32_t addr) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);
	
	if (masked_addr >= 0x1f801074 && masked_addr < 0x1f801074 + sizeof(uint32_t)) { // IRQ_STATUS
		return 0;
	}

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 0x7D000) {
		memcpy(&bytes, &bios[masked_addr & 0xfffff], sizeof(uint32_t));
		return bytes;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0xfffff], sizeof(uint32_t));
		return bytes;
	}
	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {
		return 0xffffffff;
	}
	

	printf("\nUnhandled read 0x%.8x", addr);
	exit(0);
	
	
	
}

void memory::write(uint32_t addr, uint8_t data, bool log) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);

	if (masked_addr == 0x1f802041)
		return;

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 0x7D000) {
		printf("Attempted to overwrite bios!");
		exit(0);
		return;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		ram[masked_addr & 0xfffff] = data;
		return;
	}
	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {	
		exp1[masked_addr & 0xfffff] = data;
		return;
	}
	
	printf("\nUnhandled write 0x%.8x", addr);
	exit(0);
	
	
	
}

void memory::write32(uint32_t addr, uint32_t data) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);
	if (masked_addr >= 0x1f800000 && masked_addr <= 0x1f801020) {
		return;
	}
	if (masked_addr >= 0x1f801060 && masked_addr < 0x1f801060 + sizeof(uint32_t)) {	// RAM_SIZE
		RAM_SIZE = data;
		if (debug) printf(" Write 0x%.8x to RAM_SIZE", data);
		return;
	}
	if (masked_addr >= 0x1f801070 && masked_addr < 0x1f801070 + sizeof(uint32_t)) { // IRQ_STATUS
		IRQ_STATUS = data;
		if (debug) printf(" Write 0x%.8x to IRQ_STATUS", data);
		return;
	}
	if (masked_addr >= 0x1f801074 && masked_addr < 0x1f801074 + sizeof(uint32_t)) { // IRQ_MASK
		IRQ_MASK = data;
		if (debug) printf(" Write 0x%.8x to IRQ_MASK", data);
		return;
	}
	if (masked_addr >= 0xfffe0130 && masked_addr < 0xfffe0130 + sizeof(uint32_t)) {	// CACHE_CONTROL
		CACHE_CONTROL = data;
		return;
	}
	write(addr, uint8_t(data & 0x000000ff), false);
	write(addr + 3, (data & 0xff000000) >> 24, false);
	write(addr + 2, (data & 0x00ff0000) >> 16, false);
	write(addr + 1, (data & 0x0000ff00) >> 8, false);
	
	if(debug) printf(" Write 0x%.8x at address 0x%.8x", data, addr);
}

void memory::write16(uint32_t addr, uint16_t data) {
	uint32_t masked_addr = mask_address(addr);
	if (masked_addr >= 0x1f801100 && masked_addr <= 0x1F80110B) {	// timer register
		return;
	}
	if (masked_addr >= 0x1f801d80 && masked_addr <= 0x1F801D87) {	// SPU regs
		if (debug) printf(" SPU register write, ignored", data);
		return;
	}
	write(addr, uint8_t(data & 0x00ff), false);
	write(addr + 1, (data & 0xff00) >> 8, false);
	
	if (debug) printf(" Write 0x%.4x at address 0x%.8x", read16(addr), addr);
}



void memory::loadBios() {
	
	FILE* BIOS_FILE;
	BIOS_FILE = fopen("C:\\Users\\zacse\\Downloads\\SCPH7003\\SCPH1001.bin", "rb");
	fread(bios, 1, 0x7D000, BIOS_FILE);
}

