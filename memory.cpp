#include "memory.h"
#include <fstream>
#pragma warning(disable : 4996)
memory::memory() {

}

memory::~memory() {

}

uint8_t memory::read(uint32_t addr) {
	return mem[addr];
}
uint32_t memory::read32(uint32_t addr) {
	uint32_t bytes;
	memcpy(&bytes, &mem[addr], sizeof(uint32_t));
	return (bytes);
}
void memory::loadBios() {
	
	FILE* BIOS_FILE;
	BIOS_FILE = fopen("SCPH1001.bin", "rb");
	fread(mem + 0xBFC00000, 1, 0x7D000, BIOS_FILE);
}

void memory::write(uint32_t addr, uint8_t data) {
	mem[addr] = data;
}