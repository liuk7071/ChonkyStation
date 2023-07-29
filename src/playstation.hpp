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
        mem.loadBios(biosPath);
        cpu.switchBackend(Cpu::Backend::OldInterpreter);
    }

    // Steps the system
    void step() {
        cpu.step();
    }

    u32 getPC() { return cpu.core.pc; }
    u8* getRAM() { return mem.ram; }

private:
    Cpu cpu;
    DMA dma;
    Memory mem;
    Interrupt interrupt;
    Gpu gpu;
};