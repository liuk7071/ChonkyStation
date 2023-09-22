#pragma once

#include <helpers.hpp>
#include <backends/base.hpp>


// Ciruclar dependency
class GPU;

class GPUSoftware : public GPUBackend {
public:
	GPUSoftware(GPU* gpu) : GPUBackend(gpu) {}
	void reset() override;
	u8* getVRAM() override;

	void drawTriUntextured(Vertex v0, Vertex v1, Vertex v2) override;
	void drawTriTextured(Vertex v0, Vertex v1, Vertex v2, u16 clut, u16 texpage) override;

	void drawRectUntextured(Vertex v, u16 width, u16 height) override;

	void beginTextureUpload(u16 x, u16 y, u16 width) override;
	void textureUploadData(u16 data) override;
	void endTextureUpload() override;

private:
	u8* vram = new u8[1024 * 512 * 2];	// We multiply by 2 because each pixel is 2 bytes (ABGR1555)
	inline void writePixel(u16 x, u16 y, u16 pixel);
	u16 fetchBGR555(u16 x, u16 y);
	u16 curX, curY;	// Current coordinates of the ongoing texture upload

	u16 fetch4bpp(u8 u, u8 v, u16 texpageX, u16 texpageY, u16 clutX, u16 clutY);

	// Rasterizer stuff
	s32 edgeFunction(Vertex p, Vertex v1, Vertex v2);
};