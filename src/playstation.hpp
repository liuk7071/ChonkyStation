#pragma once

#include <cpu.hpp>
#include <memory.hpp>


class PlayStation {
public:
    PlayStation(const char* biosPath) {
        mem.loadBios(biosPath);
    }
    Memory mem = Memory();
    Cpu cpu = Cpu(&mem);
};