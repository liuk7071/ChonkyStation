#pragma once

#include <helpers.hpp>


class Memory {
public:
	Memory();
	void loadBios(const char* biosPath);

	u8* ram = new u8[2MB];
	u8* scratchpad = new u8[1KB];
	u8* bios = new u8[512KB];

	template<typename T> T read(u32 addr);
};