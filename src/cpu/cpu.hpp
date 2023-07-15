#pragma once

#include <helpers.hpp>
#include <cpu_core.hpp>
#include <backends/interpreter/interpreter.hpp>
#include <memory.hpp>


class Cpu {
public:
    Cpu(Memory* mem) : mem(mem) {}

    Memory* mem;

    enum class Backend {
        Interpreter
    };
    Backend backend = Backend::Interpreter;

    CpuCore core;

    Interpreter interpreter;
    
    void (*stepFunc)(CpuCore*, Memory*) = &interpreter.step;
    void step();
};