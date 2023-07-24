#include <cpu.hpp>


void Cpu::step() {
    (*stepFunc)(&core, mem, &disassembler);
}

void Cpu::switchBackend(Backend backend) {
    switch (backend) {
    case Backend::Interpreter:    stepFunc = &interpreter.step; break;
    case Backend::OldInterpreter: stepFunc = &oldInterpreter.step; break;
    default: Helpers::panic("Unsupported backend\n");   // Should never be triggered
    }
    stepFunc = &oldInterpreter.step;
}