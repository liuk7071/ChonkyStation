#pragma once

#include <helpers.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <intc.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : intc(), mem(&intc), cpu(&mem) {
        mem.loadBios(biosPath);
    }

    // Steps the system
    void step() {
        cpu.step();
    }

private:
    Cpu cpu;
    Memory mem;
    INTC intc;
};