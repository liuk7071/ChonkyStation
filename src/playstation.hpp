#pragma once

#include <cpu.hpp>
#include <memory.hpp>
#include <helpers.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : mem(), cpu(&mem) {
        mem.loadBios(biosPath);
    }

    // Steps the system
    void step() {
        cpu.step();
    }

private:
    Memory mem;
    Cpu cpu;
};