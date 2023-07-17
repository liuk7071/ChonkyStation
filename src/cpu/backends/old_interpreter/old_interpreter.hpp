#pragma once

#include <stdint.h>
#include <cstdarg>
#include <string.h>

#include <helpers.hpp>
#include <cpu_core.hpp>
#include <memory.hpp>
#include <disassembler/disassembler.hpp>


class OldInterpreter {
public:
	static void step(CpuCore* core, Memory* mem, Disassembler* disassembler);

	void sideloadExecutable(std::string directory);
};