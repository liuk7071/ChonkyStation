#pragma once

#include <helpers.hpp>
#include <cpu_core.hpp>
#include <memory.hpp>
#include <disassembler/disassembler.hpp>

class Interpreter {
public:
	static void step(CpuCore* core, Memory* mem, Disassembler* disassembler);
};