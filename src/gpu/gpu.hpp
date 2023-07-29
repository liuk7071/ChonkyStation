#pragma once

#include <helpers.hpp>
#include <BitField.hpp>
#include <logger.hpp>


class GPU {
public:
	std::vector<u32> fifo;
	u32 getStat();
	u32 gpuRead();

	// Command processing
	void writeGp0(u32 data);
	void writeGp1(u32 data);


	enum class GP0Command {
		NOP							= 0x00,
		ClearCache					= 0x01,
		DrawModeSetting				= 0xE1,
		TextureWindowSetting		= 0xE2,
		SetDrawingAreaTopLeft		= 0xE3,
		SetDrawingAreaBottomRight	= 0xE4,
		SetDrawingOffset			= 0xE5,
		MaskBitSetting				= 0xE6
	};
	
	enum class GP1Command {
		ResetGPU				= 0x00,
		DMADirection			= 0x04,
		StartOfDisplayArea		= 0x05,
		HorizontalDisplayRange	= 0x06,
		VerticalDisplayRange	= 0x07,
		DisplayMode				= 0x08
	};


private:
	u32 stat = 0x14802000;

	bool hasCommand = false;
	u32 paramsLeft = 0;	// Parameters needed for command
	void startCommand(u32 rawCommand);

	class DrawCommand {
	public:
		DrawCommand(u32 raw);
		u32 getCommandSize();	// In words

	private:
		u32 raw;
		enum class DrawType {
			Polygon,
			Line
		} drawType;
		union Polygon {
			u32 raw;
			BitField<0, 24, u32> rgb;				// Colour of vertex 0
			BitField<24, 1, u32> rawTexture;
			BitField<25, 1, u32> semiTransparent;
			BitField<26, 1, u32> textured;
			BitField<27, 1, u32> quad;				// If it's not a quad, it's a tri
			BitField<28, 1, u32> shading;
			BitField<29, 3, u32> polygonCommand;	// Should be 0b001
		};
	};
};