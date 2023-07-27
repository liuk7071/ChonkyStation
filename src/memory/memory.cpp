#include <memory.hpp>


Memory::Memory(Interrupt* interrupt, DMA* dma, Gpu* gpu) : interrupt(interrupt), dma(dma), gpu(gpu) {
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

void Memory::loadBios(const fs::path& biosPath) {
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
u8 Memory::read(u32 vaddr) {
	const auto page = vaddr >> 16;
	const auto pointer = readTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		return *(u8*)(pointer + (vaddr & 0xffff));
	}
	
	u32 paddr = maskAddress(vaddr);

	if (Helpers::inRangeSized<u32>(paddr, 0x1f000000, 0x400)) return 0xff;
	else
		Helpers::panic("[FATAL] Unhandled read8 0x%08x (virtual 0x%08x)\n", paddr, vaddr);
}

template<>
u16 Memory::read(u32 vaddr) {
	const auto page = vaddr >> 16;
	const auto pointer = readTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		return *(u16*)(pointer + (vaddr & 0xffff));
	}

	u32 paddr = maskAddress(vaddr);

	// Interrupt
	if (paddr == 0x1f801074) return interrupt->readImask();
	// SPU
	else if (Helpers::inRangeSized<u32>(paddr, (u32)MemoryBase::SPU, (u32)MemorySize::SPU)) return 0;
	else
		Helpers::panic("[FATAL] Unhandled read16 0x%08x (virtual 0x%08x)\n", paddr, vaddr);
}

template<>
u32 Memory::read(u32 vaddr) {
	const auto page = vaddr >> 16;
	const auto pointer = readTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		return *(u32*)(pointer + (vaddr & 0xffff));
	}

	u32 paddr = maskAddress(vaddr);

	// GPU
	if (paddr == 0x1f801810) return gpu->gpuRead();
	else if (paddr == 0x1f801814) return gpu->getStat();
	// Interrupt
	else if (paddr == 0x1f801074) return interrupt->readImask();
	// DMA
	else if (Helpers::inRange<u32>(paddr, 0x1f801080, 0x1f8010e8)) {
		const auto channel = ((paddr >> 4) & 0xf) - 8;
		Helpers::assert(channel < 7, "Tried to access %dth DMA channel", channel);	// Should not get triggered

		switch (paddr & 0xf) {
		case 0x0: return dma->channels[channel].madr;
		case 0x4: return dma->channels[channel].bcr.raw;
		case 0x8: return dma->channels[channel].chcr.raw;
		default: Helpers::panic("[FATAL] Unhandled DMA read32 0x%08x\n", paddr);
		}
	}
	else if (paddr == 0x1f8010f0) return dma->dpcr;
	else if (paddr == 0x1f8010f4) return dma->dicr;
	else
		Helpers::panic("[FATAL] Unhandled read32 0x%08x (virtual 0x%08x)\n", paddr, vaddr);
}


template<>
void Memory::write(u32 vaddr, u8 data) {
	const auto page = vaddr >> 16;
	const auto pointer = writeTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		*(u8*)(pointer + (vaddr & 0xffff)) = data;
		return;
	}
	
	u32 paddr = maskAddress(vaddr);

	if (paddr == 0x1f802041) return;	// POST - External 7-segment Display (W)
	else
		Helpers::panic("[FATAL] Unhandled write8 0x%08x (virtual 0x%08x) <- 0x%02x\n", paddr, vaddr, data);
}

template<>
void Memory::write(u32 vaddr, u16 data) {
	const auto page = vaddr >> 16;
	const auto pointer = writeTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		*(u16*)(pointer + (vaddr & 0xffff)) = data;
		return;
	}

	u32 paddr = maskAddress(vaddr);

	// Interrupt
	if (paddr == 0x1f801074) interrupt->writeImask(data);
	// SPU
	else if (Helpers::inRangeSized<u32>(paddr, (u32)MemoryBase::SPU, (u32)MemorySize::SPU)) return;
	// Timers
	else if (Helpers::inRangeSized<u32>(paddr, (u32)MemoryBase::Timer, (u32)MemorySize::Timer)) return;
	else
		Helpers::panic("[FATAL] Unhandled write16 0x%08x (virtual 0x%08x) <- 0x%04x\n", paddr, vaddr, data);
}

template<>
void Memory::write(u32 vaddr, u32 data) {
	const auto page = vaddr >> 16;
	const auto pointer = writeTable[page];

	// Use fast memory if the address is in the fastmem table
	if (pointer) {
		*(u32*)(pointer + (vaddr & 0xffff)) = data;
		return;
	}

	u32 paddr = maskAddress(vaddr);

	// GPU
	if (paddr == 0x1f801810) gpu->writeGp0(data);
	else if (paddr == 0x1f801814) gpu->writeGp1(data);
	// Interrupt
	else if (paddr == 0x1f801070) interrupt->writeIstat(data);
	else if (paddr == 0x1f801074) interrupt->writeImask(data);
	// DMA
	else if (Helpers::inRange<u32>(paddr, 0x1f801080, 0x1f8010e8)) {	
		const auto channel = ((paddr >> 4) & 0xf) - 8;
		Helpers::assert(channel < 7, "Tried to access %dth DMA channel", channel);	// Should not get triggered

		switch (paddr & 0xf) {
		case 0x0: dma->channels[channel].madr	  = data; break;
		case 0x4: dma->channels[channel].bcr.raw  = data; break;
		case 0x8: {
			dma->channels[channel].chcr.raw = data;
			if (dma->channels[channel].shouldStartDMA()) {
				dma->doDMA(channel, this);
			}
			break;
		}
		default: Helpers::panic("[FATAL] Unhandled DMA write32 0x%08x <- 0x%08x\n", paddr, data);
		}
	}
	else if (paddr == 0x1f8010f0) dma->dpcr = data;
	else if (paddr == 0x1f8010f4) dma->dicr = data;
	// Timers
	else if (Helpers::inRangeSized<u32>(paddr, (u32)MemoryBase::Timer, (u32)MemorySize::Timer)) return;

	else if (paddr == 0x1f801000) return;	// Expansion 1 Base Address (usually 1F000000h)
	else if (paddr == 0x1f801004) return;	// Expansion 2 Base Address (usually 1F802000h)
	else if (paddr == 0x1f801008) return;	// Expansion 1 Delay/Size (usually 0013243Fh) (512Kbytes, 8bit bus)
	else if (paddr == 0x1f80100C) return;	// Expansion 3 Delay/Size (usually 00003022h) (1 byte)
	else if (paddr == 0x1f801010) return;	// BIOS ROM Delay/Size (usually 0013243Fh) (512Kbytes, 8bit bus)
	else if (paddr == 0x1f801014) return;	// SPU Delay/Size (200931E1h) (use 220931E1h for SPU-RAM reads)
	else if (paddr == 0x1f801018) return;	// CDROM Delay/Size (00020843h or 00020943h)
	else if (paddr == 0x1f80101C) return;	// Expansion 2 Delay/Size (usually 00070777h) (128 bytes, 8bit bus)
	else if (paddr == 0x1f801020) return;	// COM_DELAY / COMMON_DELAY (00031125h or 0000132Ch or 00001325h)
	else if (paddr == 0x1f801060) return;	// RAM_SIZE (R/W) (usually 00000B88h) (or 00000888h)
	else if (paddr == 0xfffe0130) return;	// Cache Control (R/W)
	else
		Helpers::panic("[FATAL] Unhandled write32 0x%08x (virtual 0x%08x) <- 0x%08x\n", paddr, vaddr, data);
}