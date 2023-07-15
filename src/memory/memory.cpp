#include <memory.hpp>


Memory::Memory() {
	std::memset(ram, 0, 2MB);
	std::memset(scratchpad, 0, 1KB);

	constexpr size_t pageSize = 64 * 1024; // 64KB pages
	readTable.resize(0x10000, 0);
	writeTable.resize(0x10000, 0);

	constexpr u32 PAGE_SIZE = 64KB; // Page size = 64KB
#ifndef ENABLE_RAM_MIRRORING
	constexpr auto ramPages = 32;
#else
	constexpr auto ramPages = 32 * 4;
#endif

	for (auto pageIndex = 0; pageIndex < ramPages; pageIndex++) {
		const auto pointer = (uintptr_t)&ram[(pageIndex * PAGE_SIZE) & 0x1FFFFF];
		readTable[pageIndex + 0x0000] = pointer; // KUSEG RAM
		readTable[pageIndex + 0x8000] = pointer; // KSEG0
		readTable[pageIndex + 0xA000] = pointer; // KSEG1

		writeTable[pageIndex + 0x0000] = pointer; // KUSEG RAM
		writeTable[pageIndex + 0x8000] = pointer; // KSEG0
		writeTable[pageIndex + 0xA000] = pointer; // KSEG1
	}

	for (auto pageIndex = 0; pageIndex < 8; pageIndex++) {
		const auto pointer = (uintptr_t)&bios[(pageIndex * PAGE_SIZE)];
		readTable[pageIndex + 0x1FC0] = pointer; // KUSEG
		readTable[pageIndex + 0x9FC0] = pointer; // KSEG0
		readTable[pageIndex + 0xBFC0] = pointer; // KSEG1
	}
}

void Memory::loadBios(const char* biosPath) {
	auto temp = Helpers::readBinary(biosPath);
	std::memcpy(bios, temp.data(), 512KB);
}

u32 Memory::maskAddress(u32 vaddr) {
	static const u32 ADDR_MASK[] = {
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x7FFFFFFF,	
		0x1FFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF
	};

	const auto idx = (vaddr >> 29);

	return vaddr & ADDR_MASK[idx];
}

template<>
u32 Memory::read(u32 vaddr) {
	u32 paddr = maskAddress(vaddr);

	const auto page = vaddr >> 16;
	const auto pointer = readTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		return *(u32*)(pointer + (vaddr & 0xffff));
	}

	//if (Helpers::inRangeSized(paddr, (u32)MemoryBase::BIOS, (u32)MemorySize::BIOS)) {
	//	paddr &= (u32)MemorySize::BIOS - 1;
	//	u32 data;
	//}
	//else
		Helpers::panic("[FATAL] Unhandled read32 @ 0x%08x (virtual 0x%08x)\n", paddr, vaddr);
}