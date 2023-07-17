#include "gpu.hpp"


u32 Gpu::getStat() {
	// Stubbed
	stat |= 1 << 26;	// Ready to receive cmd word
	stat |= 1 << 27;	// Ready to send VRAM to CPU
	stat |= 1 << 28;	// Ready to receive DMA block
	stat |= 1 << 30;	// DMA direction CPU -> GP0
	return stat;
}


void Gpu::writeGp0(u32 data) {
	if (!hasCommand)
		startCommand(data);
}

void Gpu::startCommand(u32 rawCommand) {
	// We handle single-word commands (i.e. all the configuration ones) in this function
	const auto cmd = (GP0Command)((rawCommand >> 24) & 0xff);
	switch (cmd) {
	case GP0Command::DrawModeSetting: {
		// Bits 0-10 are copied into GPUSTAT
		stat &= ~0x7ff;
		stat |= rawCommand & 0x7ff;
		// Bit 11 is copied into GPUSTAT.15
		stat &= ~(1 << 15);
		stat |= (rawCommand & (1 << 11)) << 4;
		break;
	}
	default:
		Helpers::panic("[GPU] Unimplemented command 0x%02x\n", (u32)cmd);
	}
}