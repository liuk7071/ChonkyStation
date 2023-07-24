#pragma once

#include <helpers.hpp>
#include <interrupt.hpp>
#include <dma.hpp>
#include <gpu.hpp>


class Memory {
public:
	Memory(Interrupt* interrupt, DMA* dma, Gpu* gpu);
    void loadBios(const fs::path& biosPath);

	u8* ram = new u8[2MB];
	u8* scratchpad = new u8[1KB];
	u8* bios = new u8[512KB];

    Interrupt* interrupt;
    DMA* dma;
    Gpu* gpu;

    // Base addresses
    enum class MemoryBase {
        RAM = 0x00000000,
        SPRAM = 0x1F800000,
        SIO = 0x1F801040,
        DMA = 0x1F801080,
        Timer = 0x1F801100,
        SPU = 0x1F801C00,
        BIOS = 0x1FC00000,
    };

    // Memory sizes
    enum class MemorySize {
        RAM = 0x200000,
        SPRAM = 0x000400,
        SIO = 0x000020,
        DMA = 0x000080,
        Timer = 0x000030,
        SPU = 0x000280,
        BIOS = 0x080000,
    };

    u32 maskAddress(u32 vaddr);
	template<typename T> T read(u32 vaddr);
    template<typename T> void write(u32 vaddr, T data);

private:
    // Software fastmem implementation
    std::vector<uptr> readTable;
    std::vector<uptr> writeTable;
};