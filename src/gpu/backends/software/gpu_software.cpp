#include <backends/software/gpu_software.hpp>


void GPUSoftware::reset() {
	std::memset(vram, 0, 1024 * 512);
}

u8* GPUSoftware::getVRAM() {
	return vram;
}