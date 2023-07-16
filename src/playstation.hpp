#pragma once

#include <helpers.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <intc.hpp>
#include <dma.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : intc(), dma(), mem(&intc, &dma), cpu(&mem) {
        mem.loadBios(biosPath);
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
};