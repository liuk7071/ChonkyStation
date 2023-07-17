#pragma once

#include <helpers.hpp>
#include <cpu_core.hpp>
#include <backends/interpreter/interpreter.hpp>
#include <backends/old_interpreter/old_interpreter.hpp>
#include <memory.hpp>


class Cpu {
public:
    Cpu(Memory* mem) : mem(mem) {
        memset(core.gprs, 0, sizeof(u32) * 32);
    }

    Memory* mem;

    enum class Backend {
        Interpreter,
        OldInterpreter
    };
    Backend backend = Backend::Interpreter;

    CpuCore core;

    Interpreter interpreter;
    OldInterpreter oldInterpreter;
    
    void (*stepFunc)(CpuCore*, Memory*, Disassembler*) = &interpreter.step;
    void switchBackend(Backend backend);

    void step();

private:
    Disassembler disassembler;
};