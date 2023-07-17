#include <cpu.hpp>


void Cpu::step() {
    (*stepFunc)(&core, mem, &disassembler);
}

void Cpu::switchBackend(Backend backend) {
    stepFunc = &oldInterpreter.step;
}