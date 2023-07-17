#pragma once

#include <helpers.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <intc.hpp>
#include <dma.hpp>
#include <gpu.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : intc(), gpu(), dma(), mem(&intc, &dma, &gpu), cpu(&mem) {
        mem.loadBios(biosPath);
        //cpu.switchBackend(Cpu::Backend::OldInterpreter);
    }

    // Steps the system
    void step() {
        cpu.step();
    }

private:
    Cpu cpu;
    DMA dma;
    Memory mem;
    INTC intc;
    Gpu gpu;
};