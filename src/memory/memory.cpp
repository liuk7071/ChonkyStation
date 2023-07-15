#include <memory.hpp>


Memory::Memory() {
	std::memset(ram, 0, 2MB);
	std::memset(scratchpad, 0, 1KB);
}

void Memory::loadBios(const char* biosPath) {
	auto temp = Helpers::readBinary(biosPath);
	std::memcpy(bios, temp.data(), 512KB);
}

template<>
u32 Memory::read(u32 addr) {
	Helpers::panic("[FATAL] Unhandled read32 @ 0x%08x\n", addr);
}