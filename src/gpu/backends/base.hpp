#pragma once

#include <helpers.hpp>
#include <backends/vertex.hpp>


// Ciruclar dependency
class GPU;

class GPUBackend {
public:
	GPUBackend(GPU* gpu) : gpu(gpu) {}
	virtual void reset() { Helpers::panic("[FATAL] GPU Backend did not define reset function\n"); };
	virtual u8* getVRAM() { Helpers::panic("[FATAL] GPU Backend did not define getVRAM function\n"); };

	// Drawing
	virtual void drawTriUntextured(Vertex v0, Vertex v1, Vertex v2) { Helpers::panic("[FATAL] GPU Backend did not define drawTriUntextured function\n"); }

	// Textures
	virtual void beginTextureUpload(u16 x, u16 y, u16 width) { Helpers::panic("[FATAL] GPU Backend did not define beginTextureUpload function\n"); };
	virtual void textureUploadData(u16 data) { Helpers::panic("[FATAL] GPU Backend did not define textureUploadData function\n"); };
	virtual void endTextureUpload() { Helpers::panic("[FATAL] GPU Backend did not define endTextureUpload function\n"); }

protected:
	GPU* gpu;

	const u16 vramWidth = 1024;
	const u16 vramHeight = 512;

	struct {
		u16 x, y, width;
	} textureUpload;
	bool uploadingTexture = false;
};