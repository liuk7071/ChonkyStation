#include <cpu.hpp>


void Cpu::step() {
    (*stepFunc)(&core, mem);
}