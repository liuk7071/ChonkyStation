/* TODO: Software fastmem
https://wheremyfoodat.github.io/software-fastmem/ */

#include "memory.h"
#include <iostream>
#define log

#pragma warning(disable : 4996)
memory::memory() {
	debug = false;
}

memory::~memory() {

}

void memory::debug_log(const char* fmt, ...) {
#ifdef log
	std::va_list args;
	va_start(args, fmt);
	std::vprintf(fmt, args);
	va_end(args);
#endif
}
void memory::debug_warn(const char* fmt, ...) {
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
	std::va_list args;
	va_start(args, fmt);
	std::vprintf(fmt, args);
	va_end(args);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
void memory::debug_err(const char* fmt, ...) {
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	std::va_list args;
	va_start(args, fmt);
	std::vprintf(fmt, args);
	va_end(args);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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


	if (masked_addr == 0x1F801070) { // I_STAT
		debug_log("[IRQ] Status 8bit read\n");
		return I_STAT;
	}

	// controller
	if (masked_addr == 0x1f801040) {	// JOY_RX_DATA
		debug_warn("[PAD] Read JOY_RX_DATA (0x%x)\n", pad1.joy_rx_data);
		return pad1.joy_rx_data;
	}

	if (masked_addr == 0x1f801800) {	// cdrom status
		debug_log("[CDROM] Status register read\n");
		return CDROM.status;
	}

	if (masked_addr == 0x1f801801) {
		switch (CDROM.status & 0b11) {
		case 1:
			debug_log("[CDROM] Read response fifo\n");
			return CDROM.read_fifo();
		default:
			debug_log("Unhandled CDROM read 0x%x index %d", addr, CDROM.status & 0b11);
			exit(0);
		}
	}

	if (masked_addr == 0x1f801803) {
		switch (CDROM.status & 0b11) {
		case 0:
			debug_log("[CDROM] Read IE\n");
			return CDROM.interrupt_enable;
		case 1:
			debug_log("[CDROM] Read IF\n");
			return CDROM.interrupt_flag;
		default:
			debug_log("Unhandled CDROM read 0x%x index %d", addr, CDROM.status & 0b11);
			exit(0);
		}
	}

	if (masked_addr >= 0x1FC00000 && masked_addr <= 0x1FC00000 + 524288) {
		memcpy(&bytes, &bios[masked_addr & 0x7ffff], sizeof(uint8_t));
		return bytes;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		memcpy(&bytes, &scratchpad[masked_addr & 0x3ff], sizeof(uint8_t));
		return bytes;
	}

	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0x1fffff], sizeof(uint8_t));
		return bytes;
	}

	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {	// return default exp1 value
		return 0xff;
	}

	debug_log("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

uint16_t memory::read16(uint32_t addr) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);


	if (masked_addr == 0x1f801120) {	// timer 2 stuff
		printf("timer\n");
		return 0;
	}

	// controllers
	if (masked_addr == 0x1f801044) { // JOY_STAT
		debug_warn("[PAD] Read JOY_STAT\n");
		//return pad1.joy_stat;
		return rand() % 0xffff;
	}

	if (masked_addr == 0x1f80104a) {	// JOY_CTRL
		uint16_t randn = rand() % 0xffff;
		debug_warn("[PAD] Read JOY_CTRL (0x%x)\n", randn);
		return pad1.joy_ctrl;
	}

	if (masked_addr == 0x1f801040) {	// JOY_RX_DATA
		debug_warn("[PAD] Read JOY_RX_DATA\n");
		return pad1.joy_rx_data;
	}

	if (masked_addr >= 0x1F801D80 && masked_addr <= 0x1F801DBC) {	// SPUSTAT
		return 0;
	}

	if (masked_addr == 0x1F801070) { // I_STAT
		debug_log("[IRQ] Status 16bit read\n");
		return I_STAT;
	}

	if (masked_addr == 0x1f801074) { // I_MASK
		debug_log("[IRQ] Status 16bit read\n");
		return I_MASK;
	}

	if (masked_addr >= 0x1F801C00 && masked_addr <= 0x1F801CfE || masked_addr == 0x1f801d0c && masked_addr <= 0x1f801dfc || masked_addr >= 0x1f801d1c && masked_addr <= 0x1f801dfc)	// more spu registers
		return 0;

	if (masked_addr >= 0x1FC00000 && masked_addr <= 0x1FC00000 + 524288) {
		memcpy(&bytes, &bios[masked_addr & 0x7ffff], sizeof(uint16_t));
		return bytes;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		memcpy(&bytes, &scratchpad[masked_addr & 0x3ff], sizeof(uint16_t));
		return bytes;
	}

	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0x1fffff], sizeof(uint16_t));
		return bytes;
	}

	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {
		return 0xffff;
	}

	debug_log("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

uint32_t memory::read32(uint32_t addr) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);


	if (masked_addr == 0x1f801110) {	// timer 1 stuff
		//printf("timer1\n");
		return 0;
	}

	if (masked_addr == 0x1f80101c) {
		return exp2_delay_size;
	}

	if (masked_addr == 0x1f801070) { // I_STAT
		return I_STAT;
	}

	if (masked_addr == 0x1f801074) { // I_MASK
		return I_MASK;
	}

	if (masked_addr == 0x1f801814) {	// GPUSTAT
		if (debug) debug_log("\n GPUSTAT read");
		return 0b01011110100000000000000000000000;		// stubbing it
	}

	if (masked_addr == 0x1f801810) // GPUREAD
		return gpuread;

	// dma
	if (masked_addr == 0x1f8010f0) 	// DCPR
		return DCPR;

	if (masked_addr == 0x1f8010f4) // DICR
		return DICR;

	// channel 2
	if (masked_addr == 0x1f8010a0) 	// base address
		return Ch2.MADR;

	if (masked_addr == 0x1f8010a4) // block control
		return Ch2.BCR;

	if (masked_addr == 0x1f8010a8) 	// control
		return Ch2.CHCR;

	// channel 3
	if (masked_addr == 0x1f8010b0) 	// base address
		return Ch3.MADR;

	if (masked_addr == 0x1f8010b4) // block control
		return Ch3.BCR;

	if (masked_addr == 0x1f8010b8) 	// control
		return Ch3.CHCR;

	// channel 6
	if (masked_addr == 0x1f8010e0) 	// base address
		return Ch6.MADR;

	if (masked_addr == 0x1f8010e4) // block control
		return Ch6.BCR;

	if (masked_addr == 0x1f8010e8)	// control
		return Ch6.CHCR;

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 524288) {
		memcpy(&bytes, &bios[masked_addr & 0x7ffff], sizeof(uint32_t));
		return bytes;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		memcpy(&bytes, &scratchpad[masked_addr & 0x3ff], sizeof(uint32_t));
		return bytes;
	}

	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		memcpy(&bytes, &ram[masked_addr & 0x1fffff], sizeof(uint32_t));
		return bytes;
	}

	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {
		return 0xffffffff;
	}

	debug_log("\nUnhandled read 0x%.8x", addr);
	exit(0);
}

void memory::write(uint32_t addr, uint8_t data, bool log) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);


	// controllers
	if (masked_addr == 0x1f801040) {
		debug_warn("[PAD] Write 0x%x to JOY_TX_DATA\n", data);
		pad1.joy_tx_data = data;
		pad1.exec();
		return;
	}

	if (masked_addr == 0x1f801800) {	// cdrom status
		debug_log("[CDROM] Write 0x%x to status register\n", data);
		CDROM.status &= ~0b11;
		CDROM.status |= data & 0b11;
		return;
	}

	if (masked_addr == 0x1f801801) {
		switch (CDROM.status & 0b11) {
		case 0:
			CDROM.execute(data);
			break;
		default:
			debug_log("Unhandled CDROM write 0x%x index %d", addr, CDROM.status & 0b11);
			exit(0);
		}

		return;
	}

	if (masked_addr == 0x1f801802) {
		switch (CDROM.status & 0b11) {
		case 0:
			CDROM.push(data);
			break;
		case 1:
			debug_log("[CDROM] Write 0x%x to interrupt enable register\n", data);
			CDROM.interrupt_enable = data;
			break;
		default:
			debug_log("Unhandled CDROM write 0x%x index %d", addr, CDROM.status & 0b11);
			exit(0);
		}
		return;
	}

	if (masked_addr == 0x1f801803) {
		switch (CDROM.status & 0b11) {
		case 0:
			debug_log("[CDROM] Write 0x%x to request register\n", data);
			CDROM.request = data;
			break;
		case 1:
			debug_log("[CDROM] Write 0x%x to interrupt flag register\n", data);
			CDROM.interrupt_flag &= ~data;
			if ((CDROM.interrupt_flag & 0b111) == 0) {
				CDROM.irq = false;
			}
			break;
		default:
			debug_log("Unhandled CDROM write 0x%x index %d", addr, CDROM.status & 0b11);
			exit(0);
		}

		return;
	}

	if (masked_addr >= 0x1f80104a && masked_addr < 0x1f80104a + sizeof(uint32_t))	// joy_ctrl
		return;

	if (masked_addr == 0x1f802080) {
		debug_log("%c", data);
		return;
	}

	if (masked_addr == 0x1f801104 || masked_addr == 0x1f801108 || masked_addr == 0x1f801100 || masked_addr == 0x1f801114 || masked_addr == 0x1f801118) {
		return;
	}

	if (masked_addr == 0x1f802041)
		return;

	if (masked_addr >= 0x1FC00000 && masked_addr < 0x1FC00000 + 0x7D000) {
		debug_log("Attempted to overwrite bios!");
		exit(0);
		return;
	}

	if (masked_addr >= 0x1f800000 && masked_addr < 0x1f800000 + 1024) {
		scratchpad[masked_addr & 0x3ff] = data;
		return;
	}

	if (masked_addr >= 0x00000000 && masked_addr < 0x00000000 + 0x200000) {
		ram[masked_addr & 0x1fffff] = data;
		return;
	}

	if (masked_addr >= 0x1F000000 && masked_addr < 0x1F000000 + 0x400) {
		exp1[masked_addr & 0xfffff] = data;
		return;
	}

	else if (masked_addr == 0x1f802082) // Exit code register for Continuous Integration tests
		exit(data);

	debug_log("\nUnhandled write 0x%.8x", addr);
	exit(0);
}

void memory::write32(uint32_t addr, uint32_t data) {
	uint32_t bytes;
	uint32_t masked_addr = mask_address(addr);


	if (masked_addr == 0x1F801070) { // I_STAT
		debug_log("[IRQ] Write 0x%x to I_STAT\n", data);
		I_STAT &= data;
		return;
	}

	if (masked_addr == 0x1f801074) { // I_MASK
		I_MASK = data;
		debug_log("[IRQ] Write 0x%x to I_MASK\n", data);
		return;
	}

	if (masked_addr == 0x1f80101c) {
		exp2_delay_size = data;
		return;
	}

	if (masked_addr == 0x1f801800)
		return;

	if (masked_addr == 0x1f8010f0) { // DCPR
		DCPR = data;
		if (debug) debug_log(" Write 0x%.8x to dcpr", data);
		return;
	}

	if (masked_addr == 0x1f8010f4) { // DICR
		DICR = data;
		if (debug) debug_log(" Write 0x%.8x to dicr", data);
		return;
	}

	// channel 2
	if (masked_addr == 0x1f8010a0) {	// base address
		Ch2.MADR = data;
		return;
	}

	if (masked_addr == 0x1f8010a4) { // block control
		Ch2.BCR = data;
		return;
	}

	if (masked_addr == 0x1f8010a8) {	// control
		Ch2.CHCR = data;
		return;
	}

	// channel 3
	if (masked_addr == 0x1f8010b0) {	// base address
		Ch3.MADR = data;
		return;
	}

	if (masked_addr == 0x1f8010b4) { // block control
		Ch3.BCR = data;
		return;
	}

	if (masked_addr == 0x1f8010b8) {	// control
		Ch3.CHCR = data;
		return;
	}

	// channel 6
	if (masked_addr == 0x1f8010e0) {	// base address
		Ch6.MADR = data;
		return;
	}

	if (masked_addr == 0x1f8010e4) { // block control
		Ch6.BCR = data;
		return;
	}

	if (masked_addr == 0x1f8010e8) {	// control
		Ch6.CHCR = data;
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
		if (debug) debug_log(" Write 0x%.8x to RAM_SIZE", data);
		return;
	}

	if (masked_addr >= 0xfffe0130 && masked_addr < 0xfffe0130 + sizeof(uint32_t)) {	// CACHE_CONTROL
		CACHE_CONTROL = data;
		return;
	}

	write(addr, uint8_t(data & 0x000000ff), false);
	write(addr + 3, uint8_t((data & 0xff000000) >> 24), false);
	write(addr + 2, uint8_t((data & 0x00ff0000) >> 16), false);
	write(addr + 1, uint8_t((data & 0x0000ff00) >> 8), false);

	if (debug) debug_log(" Write 0x%.8x at address 0x%.8x", data, addr);
}

void memory::write16(uint32_t addr, uint16_t data) {
	uint32_t masked_addr = mask_address(addr);


	if (masked_addr == 0x1F801070) { // I_STAT
		debug_log("[IRQ] Write 0x%x to I_STAT\n", data);
		I_STAT &= data;
		return;
	}

	if (masked_addr == 0x1F801074) { // I_MASK
		debug_log("[IRQ] Write 0x%x to I_MASK\n", data);
		I_MASK = data;
		return;
	}

	if (masked_addr == 0x1f802082) // "its a PCSX register, ignore it"
		return;

	// controller
	if (masked_addr == 0x1f801048) {
		debug_warn("[PAD] Write 0x%x to JOY_MODE\n");
		pad1.joy_mode = data;
		return;
	}

	if (masked_addr == 0x1f80104e) {
		debug_warn("[PAD] Write 0x%x to JOY_BAUD\n");
		pad1.joy_baud = data;
		return;
	}

	if (masked_addr == 0x1f801104 || masked_addr == 0x1f801108 || masked_addr == 0x1f801100 || masked_addr == 0x1f801114 || masked_addr == 0x1f801118 || masked_addr == 0x1f801110 || masked_addr == 0x1f801124 || masked_addr == masked_addr == 0x1f801124 || masked_addr == 0x1f801128 || masked_addr == 0x1f801120) {
		return;
	}

	if (masked_addr >= 0x1F801C00 && masked_addr <= 0x1F801E80) {	// SPU regs
		if (debug) debug_log(" SPU register write, ignored");
		return;
	}

	write(addr, uint8_t(data & 0x00ff), false);
	write(addr + 1, (data & 0xff00) >> 8, false);

	if (debug) debug_log(" Write 0x%.4x at address 0x%.8x", read16(addr), addr);
}


static auto readBinary(std::string directory) -> std::vector<uint8_t> {
	std::ifstream file(directory, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Couldn't find ROM at " << directory << "\n";
		//exit(1);
	}

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

void memory::loadBios(std::string directory) {
	bios = readBinary(directory);
}

uint32_t memory::loadExec(std::string directory) {
	file = readBinary(directory);

	uint32_t start_pc;
	uint32_t entry_addr;
	uint32_t file_size;

	memcpy(&start_pc, &file[0x10], sizeof(uint32_t));
	memcpy(&entry_addr, &file[0x18], sizeof(uint32_t));
	memcpy(&file_size, &file[0x1c], sizeof(uint32_t));

	debug_log("\nStart pc: 0x%x", start_pc);
	debug_log("\nDestination: 0x%x", entry_addr);
	debug_log("\nFile size: 0x%x\n\n\n", file_size);

	for (int i = 0; i < file_size; i++) {
		write(entry_addr + i, file[0x800 + i], false);
	}

	return start_pc;
}