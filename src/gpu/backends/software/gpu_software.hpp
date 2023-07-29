#pragma once

#include <helpers.hpp>
#include <backends/base.hpp>


class GPUSoftware : public GPUBackend {
public:
	void reset() override;
	u8* getVRAM() override;

	void beginTextureUpload(u16 x, u16 y, u16 width) override;
	void textureUploadData(u16 data) override;
	void endTextureUpload() override;

private:
	u8* vram = new u8[1024 * 512 * 2];	// We multiply by 2 because each pixel is 2 bytes (ABGR1555)
	inline void writePixel(u16 x, u16 y, u16 pixel);
	u16 curX, curY;	// Current coordinates of the ongoing texture upload
};