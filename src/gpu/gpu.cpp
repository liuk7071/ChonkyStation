#include "gpu.hpp"


u32 Gpu::getStat() {
	// Stubbed
	stat |= 1 << 26;	// Ready to receive cmd word
	stat |= 1 << 27;	// Ready to send VRAM to CPU
	stat |= 1 << 28;	// Ready to receive DMA block
	stat |= 1 << 30;	// DMA direction CPU -> GP0
	return stat;
}

u32	Gpu::gpuRead() {
	// Stubbed
	return 0;
}

void Gpu::writeGp0(u32 data) {
	if (!hasCommand)
		startCommand(data);
}

void Gpu::writeGp1(u32 data) {
	const auto cmd = (data >> 24) & 0xff;

	switch (cmd) {
	case (u32)GP1Command::ResetGpu: {
		stat = 0x14802000;
		break;
	}
	case (u32)GP1Command::DMADirection: {
		// Bits 0-1 are copied to GPUSTAT.29-30
		stat &= ~(3 << 29);
		stat |= (data & 3) << 29;
		break;
	}
	case (u32)GP1Command::DisplayMode: {
		// Bits 0-5 are copied to GPUSTAT.17-22
		stat &= ~(0x3f << 17);
		stat |= (data & 0x3f) << 17;
		// Bit 6 is copied to GPUSTAT.16
		stat &= ~(1 << 16);
		stat |= (data & (1 << 6)) << 10;
		// Bit 7 is copied to GPUSTAT.17
		stat &= ~(1 << 14);
		stat |= (data & (1 << 7)) << 7;
		break;
	}
	default:
		Helpers::panic("[GPU] Unimplemented gp1 command 0x%02x\n", cmd);
	}
}

void Gpu::startCommand(u32 rawCommand) {
	// We handle single-word commands (i.e. all the configuration ones) in this function
	const auto cmd = (rawCommand >> 24) & 0xff;
	switch (cmd) {
	case (u32)GP0Command::NOP: {
		// NOP
		break;
	}
	case (u32)GP0Command::DrawModeSetting: {
		// Bits 0-10 are copied into GPUSTAT
		stat &= ~0x7ff;
		stat |= rawCommand & 0x7ff;
		// Bit 11 is copied into GPUSTAT.15
		stat &= ~(1 << 15);
		stat |= (rawCommand & (1 << 11)) << 4;
		break;
	}
	default:
		Helpers::panic("[GPU] Unimplemented gp0 command 0x%02x\n", (u32)cmd);
	}
}