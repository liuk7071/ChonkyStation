#pragma once

#include <helpers.hpp>
#include <BitField.hpp>
#include <logger.hpp>
#include <backends/software/gpu_software.hpp>
#include <backends/base.hpp>
#include <scheduler.hpp>


namespace GPUConstants {
// TODO: PAL?
constexpr u64 cyclesPerHdraw = 2560 / 1.57;
constexpr u64 cyclesPerScanline = 3413 / 1.57;	// NTSC
constexpr u64 scanlinesPerVdraw = 240;
constexpr u64 scanlinesPerFrame = 263;
} // End namespace GPUConstants

class GPU {
public:
	GPU(Scheduler* scheduler) : software(this), scheduler(scheduler) {
		backend = &software;
	}

	Scheduler* scheduler;

	u64 currentScanline = 0;
	static void scanlineEvent(void* classptr);
	bool vblank = false;

	bool uploadingTexture = false;
	std::vector<u32> fifo;
	u32 getStat();
	u32 gpuRead();
	u8* getVRAM() { return backend->getVRAM(); };

	// Command processing
	void writeGp0(u32 data);
	void writeGp1(u32 data);


	enum class GP0Command {
		NOP							= 0x00,
		ClearCache					= 0x01,
		UploadTexture				= 0xA0,
		ReadVRAM					= 0xC0,
		DrawModeSetting				= 0xE1,
		TextureWindowSetting		= 0xE2,
		SetDrawingAreaTopLeft		= 0xE3,
		SetDrawingAreaBottomRight	= 0xE4,
		SetDrawingOffset			= 0xE5,
		MaskBitSetting				= 0xE6
	};
	
	enum class GP1Command {
		ResetGPU				= 0x00,
		ResetCommandBuffer		= 0x01,
		AcknowledgeIRQ1			= 0x02,
		DisplayEnable			= 0x03,
		DMADirection			= 0x04,
		StartOfDisplayArea		= 0x05,
		HorizontalDisplayRange	= 0x06,
		VerticalDisplayRange	= 0x07,
		DisplayMode				= 0x08
	};

private:
	u32 stat = 0x14802000;

	GPUBackend* backend;
	GPUSoftware software;

	bool hasCommand = false;
	u32 paramsLeft = 0;	// Parameters needed for command
	void startCommand(u32 rawCommand);

	class DrawCommand {
	public:
		DrawCommand(u32 raw);
		u32 getCommandSize();	// In words
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
			BitField<28, 1, u32> shaded;
			BitField<29, 3, u32> polygonCommand;	// Should be 0b001
		};

		Polygon getPolygon();

	private:
		u32 raw;
	};
};