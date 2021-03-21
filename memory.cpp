#include "memory.h"
#include <fstream>

#pragma warning(disable : 4996)
memory::memory() {
	debug = false;
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

	if (masked_addr >= 0x1FC00000 && masked_addr <= 0x1FC00000 + 524288) {
		memcpy(&bytes, &bios[masked_addr & 0x7ffff], sizeof(uint8_t));
		return bytes;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		memcpy(&bytes, &scratchpad[masked_addr & 0x400], sizeof(uint8_t));
		return bytes;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0x1fffff], sizeof(uint8_t));
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

	if (masked_addr >= 0x1F801D80 && masked_addr <= 0x1F801DBC) {	// SPUSTAT
		return 0;
	}

	if (masked_addr >= 0x1f801074 && masked_addr <= 0x1f801074 + sizeof(uint32_t)) { // IRQ_STATUS
		return 0;
	}

	if (masked_addr >= 0x1F801C00 && masked_addr <= 0x1F801CfE || masked_addr == 0x1f801d0c && masked_addr <= 0x1f801dfc || masked_addr >= 0x1f801d1c && masked_addr <= 0x1f801dfc)	// more spu registers
		return 0;

	if (masked_addr >= 0x1FC00000 && masked_addr <= 0x1FC00000 + 524288) {
		memcpy(&bytes, &bios[masked_addr & 0x7ffff], sizeof(uint16_t));
		return bytes;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		memcpy(&bytes, &scratchpad[masked_addr & 0x400], sizeof(uint16_t));
		return bytes;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0x1fffff], sizeof(uint16_t));
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

	if (masked_addr == 0x1f80101c) {
		return exp2_delay_size;
	}

	if (masked_addr >= 0x1F801080 && masked_addr <= 0x1F8010FC) // dma registers
		return 0;

	if (masked_addr == 0x1f801814) {	// GPUSTAT
		if(debug) printf("\n GPUSTAT read");
		return gpustat;		// stubbing it
	}
	if (masked_addr == 0x1f801810) // GPUREAD
		return gpuread;
	

	// dma

	if (masked_addr == 0x1f8010f0) 	// DCPR
		return DCPR;
	if (masked_addr == 0x1f8010f4) // DICR
		return DICR;

	// channel 2
	if (masked_addr == 0x1f8010a8) 	// control
		return channel2_control;

	// channel 6
	if (masked_addr == 0x1f8010e0) 	// base address
		return channel6_base_address;
	if (masked_addr == 0x1f8010e4) // block control
		return channel6_block_control;
	if (masked_addr == 0x1f8010e8)	// control
		return channel6_control;	
	

	if (masked_addr >= 0x1f801074 && masked_addr <= 0x1f801074 + sizeof(uint32_t)) { // IRQ_STATUS
		return 0;
	}

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 524288) {
		memcpy(&bytes, &bios[masked_addr & 0x7ffff], sizeof(uint32_t));
		return bytes;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		memcpy(&bytes, &scratchpad[masked_addr & 0x400], sizeof(uint32_t));
		return bytes;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0x1fffff], sizeof(uint32_t));
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

	if (masked_addr == 0x1f802080) {
		printf("%c", data);
		return;
	}

	if (masked_addr == 0x1f801104 || masked_addr == 0x1f801108 || masked_addr == 0x1f801100 || masked_addr == 0x1f801114 || masked_addr == 0x1f801118) {
		return;
	}
	if (masked_addr == 0x1f802041)
		return;

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 0x7D000) {
		printf("Attempted to overwrite bios!");
		exit(0);
		return;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		scratchpad[masked_addr & 0x400] = data;
	}
	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		ram[masked_addr & 0x1fffff] = data;
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

	if (masked_addr == 0x1f80101c) {
		exp2_delay_size = data;
		return;
	}

	if (masked_addr == 0x1f8010f0) { // DCPR
		DCPR = data;
		if (debug) printf(" Write 0x%.8x to dcpr", data);
		return;
	}
	if (masked_addr == 0x1f8010f4) { // DICR
		DICR = data;
		if (debug) printf(" Write 0x%.8x to dicr", data);
		return;
	}

	// channel 2
	if (masked_addr == 0x1f8010a8) {	// control
		channel2_control = data;
		return;
	}

	// channel 6
	if (masked_addr == 0x1f8010e0) {	// base address
		channel6_base_address = data;
		return;
	}
	if (masked_addr == 0x1f8010e4) { // block control
		channel6_block_control = data;
		return;
	}
	if (masked_addr == 0x1f8010e8) {	// control
		channel6_control = data;
		return;
	}

	if (masked_addr == 0x1f801104 || masked_addr == 0x1f801108 || masked_addr == 0x1f801100 || masked_addr == 0x1f801114 || masked_addr == 0x1f801118) {
		return;
	}
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

	if (masked_addr >= 0x1f801074 && masked_addr < 0x1f801074 + sizeof(uint32_t)) { // IRQ_STATUS
		IRQ_STATUS &= uint32_t(data);
		if (debug) printf(" Write 0x%.8x to IRQ_STATUS", data);
		return;
	}

	if (masked_addr == 0x1f801104 || masked_addr == 0x1f801108 || masked_addr == 0x1f801100 || masked_addr == 0x1f801114 || masked_addr == 0x1f801118 || masked_addr == 0x1f801110 || masked_addr == 0x1f801124 || masked_addr == masked_addr == 0x1f801124 || masked_addr == 0x1f801128 || masked_addr == 0x1f801120) {
		return;
	}
	if (masked_addr >= 0x1F801C00 && masked_addr <= 0x1F801E80) {	// SPU regs
		if (debug) printf(" SPU register write, ignored", data);
		return;
	}
	write(addr, uint8_t(data & 0x00ff), false);
	write(addr + 1, (data & 0xff00) >> 8, false);
	
	if (debug) printf(" Write 0x%.4x at address 0x%.8x", read16(addr), addr);
}




void memory::loadBios() {
	
	FILE* BIOS_FILE;
	BIOS_FILE = fopen("./SCPH1001.bin", "rb");
	fread(bios, 1, 524288, BIOS_FILE);
}

//template <typename C>
//std::vector<C> readExec() {
//	std::basic_ifstream<C> file{ "C:\\Users\\zacse\\Downloads\\psxtest_cpu_1\\psxtest_cpu.exe" };
//	return { std::istreambuf_iterator<C>{file}, {} };
//}

static auto readExec(std::string directory) -> std::vector<uint8_t> {
	std::ifstream file(directory, std::ios::binary);
	std::vector<uint8_t> exe;
	file.unsetf(std::ios::skipws);
	std::streampos fileSize;
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	
	exe.insert(exe.begin(),
		std::istream_iterator<uint8_t>(file),
		std::istream_iterator<uint8_t>());
	file.close();
	return exe;
}

uint32_t memory::loadExec() {
	file = readExec("C:\\Users\\zacse\\Downloads\\psxtest_cpu_1\\psxtest_cpu.exe");

	uint32_t start_pc;
	uint32_t entry_addr;
	uint32_t file_size;

	memcpy(&start_pc, &file[0x10], sizeof(uint32_t));
	memcpy(&entry_addr, &file[0x18], sizeof(uint32_t));
	memcpy(&file_size, &file[0x1c], sizeof(uint32_t));

	printf("\nStart pc: 0x%x", start_pc);
	printf("\nDestination: 0x%x", entry_addr);
	printf("\nFile size: 0x%x\n\n\n", file_size);
	
	uint8_t* data = file.data();
	
	for (int i = 0; i < file_size; i++) {
		write(entry_addr + i, data[0x800 + i], false);
	}

	
	return start_pc;
}
