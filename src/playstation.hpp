#pragma once

#include <helpers.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <interrupt.hpp>
#include <dma.hpp>
#include <gpu.hpp>
#include <cdrom.hpp>
#include <scheduler.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : cdrom(), interrupt(), gpu(&scheduler), dma(), mem(&interrupt, &dma, &gpu, &cdrom), cpu(&mem) {
        mem.loadBios(biosPath);
        cpu.switchBackend(Cpu::Backend::Interpreter);

        // Setup GPU scheduler events
        scheduler.push(&gpu.scanlineEvent, scheduler.time + GPUConstants::cyclesPerScanline, &gpu);
    }

    u64 cycles = 0;
    // Steps the system
    void step() {
        cpu.step();
        auto cyclesToAdd = isInBIOS() ? 20 : 2;
        scheduler.tick(cyclesToAdd);
        cycles += cyclesToAdd;
    }

    u32 getPC() { return cpu.core.pc; }
    u8* getRAM() { return mem.ram; }
    u8* getVRAM() { return gpu.getVRAM(); }
    bool getVBLANKAndClear() {
        bool temp = gpu.vblank;
        gpu.vblank = false;
        return temp;
    }
    void VBLANK() { interrupt.raiseInterrupt(Interrupt::InterruptType::VBLANK); }
    bool isInBIOS() { return Helpers::inRangeSized<u32>(cpu.core.pc, (u32)Memory::MemoryBase::BIOS, (u32)Memory::MemorySize::BIOS); }

private:
    Cpu cpu;
    Scheduler scheduler;
    DMA dma;
    Memory mem;
    Interrupt interrupt;
    GPU gpu;
    CDROM cdrom;
};