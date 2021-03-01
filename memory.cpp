#include "memory.h"
#include <fstream>
#pragma warning(disable : 4996)
memory::memory() {
	debug = true;
}

memory::~memory() {

}

uint8_t memory::read(uint32_t addr) {
	uint32_t bytes;
	auto shifted_addr = addr << 4;
	shifted_addr >>= 4;
	//printf("0x%.8x | 0x%.8x\n", addr, shifted_addr);

	if (addr >= 0x1FC00000 && addr < 0x1FC00000 + 0x7D000 || addr >= 0x9FC00000 && addr < 0x9FC00000 + 0x7D000 || addr >= 0xBFC00000 && addr < 0xBFC00000 + 0x7D000) {
		memcpy(&bytes, &bios[shifted_addr], sizeof(uint8_t));
		return bytes;
	}
	if (addr >= 0x00000000 && addr < 0x00000000 + 0x200000 || addr >= 0x80000000 && addr < 0x80000000 + 0x200000 || addr >= 0xA0000000 && addr < 0xA0000000 + 0x200000) {
		memcpy(&bytes, &ram[shifted_addr], sizeof(uint8_t));
		return bytes;
	}
	if (addr >= 0x1F000000 && addr < 0x1F000000 + 0x400 || addr >= 0x9F000000 && addr < 0x9F000000 + 0x400 || addr >= 0xBF000000 && addr < 0xBF000000 + 0x400) {	// return default exp1 value
		return 0xff;
	}
	if (addr >= 0x1F801000 && addr < 0x1F801000 + 0x1F40 || addr >= 0x9F801000 && addr < 0x9F801000 + 0x1F40 || addr >= 0xBF801000 && addr < 0xBF801000 + 0x1F40) {
		memcpy(&bytes, &regs[shifted_addr], sizeof(uint8_t));
		return bytes;
	}
	if (addr >= 0xFFFE0000 && addr <= 0xFFFE0000 + 500) {
		memcpy(&bytes, &mem[addr], sizeof(uint8_t));
		return bytes;
	}
	if (addr >= 0x80200000 && addr <= 0x80300000)
		return 0;

	printf("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

uint16_t memory::read16(uint32_t addr) {
	uint32_t bytes;
	auto shifted_addr = addr << 4;
	shifted_addr >>= 4;
	//printf("0x%.8x | 0x%.8x\n", addr, shifted_addr);

	if (addr >= 0x1FC00000 && addr < 0x1FC00000 + 0x7D000 || addr >= 0x9FC00000 && addr < 0x9FC00000 + 0x7D000 || addr >= 0xBFC00000 && addr < 0xBFC00000 + 0x7D000) {
		memcpy(&bytes, &bios[shifted_addr], sizeof(uint16_t));
		return bytes;
	}
	if (addr >= 0x00000000 && addr < 0x00000000 + 0x200000 || addr >= 0x80000000 && addr < 0x80000000 + 0x200000 || addr >= 0xA0000000 && addr < 0xA0000000 + 0x200000) {
		memcpy(&bytes, &ram[shifted_addr], sizeof(uint16_t));
		return bytes;
	}
	if (addr >= 0x1F000000 && addr < 0x1F000000 + 0x400 || addr >= 0x9F000000 && addr < 0x9F000000 + 0x400 || addr >= 0xBF000000 && addr < 0xBF000000 + 0x400) {	// return default exp1 value
		return 0xffff;
	}
	if (addr >= 0x1F801000 && addr < 0x1F801000 + 0x1F40 || addr >= 0x9F801000 && addr < 0x9F801000 + 0x1F40 || addr >= 0xBF801000 && addr < 0xBF801000 + 0x1F40) {
		memcpy(&bytes, &regs[shifted_addr], sizeof(uint16_t));
		return bytes;
	}
	if (addr >= 0xFFFE0000 && addr <= 0xFFFE0000 + 500) {
		memcpy(&bytes, &mem[addr], sizeof(uint16_t));
		return bytes;
	}

	printf("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

uint32_t memory::read32(uint32_t addr) {
	uint32_t bytes;
	auto shifted_addr = addr << 4;
	shifted_addr >>= 4;
	//printf("0x%.8x | 0x%.8x\n", addr, shifted_addr);

	if (addr >= 0x1FC00000 && addr < 0x1FC00000 + 0x7D000 || addr >= 0x9FC00000 && addr < 0x9FC00000 + 0x7D000 || addr >= 0xBFC00000 && addr < 0xBFC00000 + 0x7D000) {
		memcpy(&bytes, &bios[shifted_addr], sizeof(uint32_t));
		return bytes;
	}
	if (addr >= 0x00000000 && addr < 0x00000000 + 0x200000 || addr >= 0x80000000 && addr < 0x80000000 + 0x200000 || addr >= 0xA0000000 && addr < 0xA0000000 + 0x200000) {
		memcpy(&bytes, &ram[shifted_addr], sizeof(uint32_t));
		return bytes;
	}
	if (addr >= 0x1F000000 && addr < 0x1F000000 + 0x400 || addr >= 0x9F000000 && addr < 0x9F000000 + 0x400 || addr >= 0xBF000000 && addr < 0xBF000000 + 0x400) {	// return default exp1 value
		return 0xffffffff;
	}
	if (addr >= 0x1F801000 && addr < 0x1F801000 + 0x1F40 || addr >= 0x9F801000 && addr < 0x9F801000 + 0x1F40 || addr >= 0xBF801000 && addr < 0xBF801000 + 0x1F40) {
		memcpy(&bytes, &regs[shifted_addr], sizeof(uint32_t));
		return bytes;
	}
	if (addr >= 0xFFFE0000 && addr <= 0xFFFE0000 + 500) {
		memcpy(&bytes, &mem[addr], sizeof(uint32_t));
		return bytes;
	}

	printf("\nUnhandled read 0x%.8x", addr);
	exit(0);
	
	
	
}

void memory::write(uint32_t addr, uint8_t data, bool log) {
	auto shifted_addr = addr << 4;
	shifted_addr >>= 4;

	if (addr >= 0x1FC00000 && addr < 0x1FC00000 + 0x7D000 || addr >= 0x9FC00000 && addr < 0x9FC00000 + 0x7D000 || addr >= 0xBFC00000 && addr < 0xBFC00000 + 0x7D000) {
		printf("Attempted to overwrite bios!");
		exit(0);
		return;
	}
	if (addr >= 0x00000000 && addr < 0x00000000 + 0x200000 || addr >= 0x80000000 && addr < 0x80000000 + 0x200000 || addr >= 0xA0000000 && addr < 0xA0000000 + 0x200000) {
		ram[shifted_addr] = data;
		return;
	}
	if (addr >= 0x1F000000 && addr < 0x1F000000 + 0x400 || addr >= 0x9F000000 && addr < 0x9F000000 + 0x400 || addr >= 0xBF000000 && addr < 0xBF000000 + 0x400) {	// return default exp1 value
		exp1[shifted_addr] = data;
		return;
	}
	if (addr >= 0x1F801000 && addr < 0x1F801000 + 0x1F40 || addr >= 0x9F801000 && addr < 0x9F801000 + 0x1F40 || addr >= 0xBF801000 && addr < 0xBF801000 + 0x1F40) {
		regs[shifted_addr] = data;
		return;
	}
	if (addr >= 0xFFFE0000 && addr <= 0xFFFE0000 + 500) {
		mem[addr] = data;
		if (debug && log) printf(" Write 0x%.2x at address 0x%.8x", read(addr), addr);
		return;
	}
	printf("\nUnhandled write 0x%.8x", addr);
	exit(0);
	
	
	
}

void memory::write32(uint32_t addr, uint32_t data) {
	write(addr + 3, (data & 0xff000000) >> 24, false);
	write(addr + 2, (data & 0x00ff0000) >> 16, false);
	write(addr + 1, (data & 0x0000ff00) >> 8, false);
	write(addr, uint8_t(data & 0x000000ff), false);
	if(debug) printf(" Write 0x%.8x at address 0x%.8x", data, addr);
}

void memory::write16(uint32_t addr, uint16_t data) {
	write(addr + 1, (data & 0xff00) >> 8, false);
	write(addr, uint8_t(data & 0x00ff), false);
	if (debug) printf(" Write 0x%.4x at address 0x%.8x", read16(addr), addr);
}



void memory::loadBios() {
	
	FILE* BIOS_FILE;
	BIOS_FILE = fopen("C:\\Users\\zacse\\Downloads\\SCPH7003\\SCPH1001.bin", "rb");	// need to implement in frontend
	fread(bios + 0xfc00000, 1, 0x7D000, BIOS_FILE);
}

