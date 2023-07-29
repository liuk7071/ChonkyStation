#pragma once

#include <helpers.hpp>
#include <logger.hpp>


class Gpu {
public:
	std::vector<u32> fifo;
	u32 getStat();
	u32 gpuRead();

	// Command processing
	void writeGp0(u32 data);
	void writeGp1(u32 data);

	bool hasCommand = false;
	u32 paramsLeft = 0;

	enum class GP0Command {
		NOP				= 0x00,
		DrawModeSetting = 0xE1
	};
	
	enum class GP1Command {
		ResetGpu		= 0x00,
		DMADirection	= 0x04,
		DisplayMode		= 0x08
	};

	void startCommand(u32 rawCommand);

private:
	u32 stat = 0x14802000;

	MAKE_LOG_FUNCTION(log, gpuLogger)
};