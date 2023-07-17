#pragma once

#include <helpers.hpp>


class Gpu {
public:
	std::vector<u32> fifo;
	u32 getStat();

	// Command processing
	void writeGp0(u32 data);

	bool hasCommand = false;
	u32 paramsLeft = 0;

	enum class GP0Command {
		DrawModeSetting = 0xE1
	};
	
	void startCommand(u32 rawCommand);

private:
	u32 stat;
};