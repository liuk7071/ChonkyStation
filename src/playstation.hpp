#pragma once

#include <helpers.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <interrupt.hpp>
#include <dma.hpp>
#include <gpu.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : interrupt(), gpu(), dma(), mem(&interrupt, &dma, &gpu), cpu(&mem) {
#ifndef __ANDROID__
        mem.loadBios(biosPath);
#endif
        cpu.switchBackend(Cpu::Backend::OldInterpreter);
    }

    // Steps the system
    void step() {
        cpu.step();
    }

    u32 getPC() { return cpu.core.pc; }
    u8* getBIOS() { return mem.bios; }
    u8* getRAM() { return mem.ram; }
    u8* getVRAM() { return gpu.getVRAM(); }

private:
    Cpu cpu;
    DMA dma;
    Memory mem;
    Interrupt interrupt;
    GPU gpu;
};