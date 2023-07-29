#pragma once

#include <helpers.hpp>
#include <backends/vertex.hpp>


class GPUBackend {
public:
	virtual void reset() { Helpers::panic("[FATAL] GPU Backend did not define reset function\n"); };
	virtual u8* getVRAM() { Helpers::panic("[FATAL] GPU Backend did not define getVRAM function\n"); };

	// Drawing
	virtual void drawTris(Vertex* v, u64 count) { Helpers::panic("[FATAL] GPU Backend did not define drawTris function\n"); }

	// Textures
	virtual void beginTextureUpload(u16 x, u16 y, u16 width) { Helpers::panic("[FATAL] GPU Backend did not define beginTextureUpload function\n"); };
	virtual void textureUploadData(u16 data) { Helpers::panic("[FATAL] GPU Backend did not define textureUploadData function\n"); };
	virtual void endTextureUpload() { Helpers::panic("[FATAL] GPU Backend did not define endTextureUpload function\n"); }

protected:
	const u16 vramWidth = 1024;
	const u16 vramHeight = 512;

	struct {
		u16 x, y, width;
	} textureUpload;
	bool uploadingTexture = false;
};