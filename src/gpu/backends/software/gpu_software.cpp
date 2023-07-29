#include <backends/software/gpu_software.hpp>


void GPUSoftware::reset() {
	std::memset(vram, 0, 1024 * 512);
}

u8* GPUSoftware::getVRAM() {
	return vram;
}

void GPUSoftware::beginTextureUpload(u16 x, u16 y, u16 width) {
	if (uploadingTexture) Helpers::panic("[FATAL] Attempted to start a GPU texture transfer before another transfer ended\n");

	textureUpload = { x, y, width };
	curX = x;
	curY = y;
	uploadingTexture = true;
}

void GPUSoftware::textureUploadData(u16 data) {
	if (!uploadingTexture) Helpers::panic("[FATAL] Attempted to upload texture data before starting a texture transfer\n");

	writePixel(curX, curY, data);

	curX++;
	if (curX >= textureUpload.x + textureUpload.width) {
		curX = textureUpload.x;
		curY++;
	}
}

void GPUSoftware::endTextureUpload() {
	uploadingTexture = false;
}


inline void GPUSoftware::writePixel(u16 x, u16 y, u16 pixel) {
	// We multiply by 2 because we store VRAM as a u8 array, but pixels are u16s
	x *= 2;
	y *= 2;
	auto index = x + y * vramWidth;
	vram[index + 0] = pixel & 0xff;
	vram[index + 1] = pixel >> 16;
}