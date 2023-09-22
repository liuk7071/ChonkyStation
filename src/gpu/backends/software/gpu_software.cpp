#include <backends/software/gpu_software.hpp>
#include <gpu.hpp>


void GPUSoftware::reset() {
	std::memset(vram, 0, 1024 * 512);
}

u8* GPUSoftware::getVRAM() {
	return vram;
}

void GPUSoftware::beginTextureUpload(u16 x, u16 y, u16 width) {
	if (uploadingTexture) Helpers::panic("[  FATAL  ] Attempted to start a GPU texture transfer before another transfer ended\n");

	textureUpload = { x, y, width };
	curX = x;
	curY = y;
	uploadingTexture = true;
}

void GPUSoftware::textureUploadData(u16 data) {
	if (!uploadingTexture) Helpers::panic("[  FATAL  ] Attempted to upload texture data before starting a texture transfer\n");

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
	vram[index + 1] = pixel >> 8;
}

u16 GPUSoftware::fetchBGR555(u16 x, u16 y) {
	x *= 2;
	y *= 2;
	auto index = x + y * vramWidth;
	return vram[index] | (vram[index + 1] << 8);
}

u16 GPUSoftware::fetch4bpp(u8 x, u8 y, u16 texpageX, u16 texpageY, u16 clutX, u16 clutY) {
	u16 tex = fetchBGR555(x / 4 + texpageX, y + texpageY);
	auto idx = (tex >> ((x % 4) * 4)) & 0xf;
	return fetchBGR555(clutX + idx, clutY);
}

s32 GPUSoftware::edgeFunction(Vertex v0, Vertex v1, Vertex p) {
	return (v1.x - v0.x) * (p.y - v0.y) - (v1.y - v0.y) * (p.x - v0.x);
}

void GPUSoftware::drawTriUntextured(Vertex v0, Vertex v1, Vertex v2) {
	auto xMin = std::min({ v0.x, v1.x, v2.x });
	auto yMin = std::min({ v0.y, v1.y, v2.y });
	auto xMax = std::max({ v0.x, v1.x, v2.x });
	auto yMax = std::max({ v0.y, v1.y, v2.y });

	if (edgeFunction(v0, v1, v2) < 0) {
		std::swap(v1, v2);
	}

	Vertex p;

	const auto area = edgeFunction(v0, v1, v2);

	for (p.y = yMin; p.y < yMax; p.y++) {
		for (p.x = xMin; p.x < xMax; p.x++) {
			s32 w0 = edgeFunction(v1, v2, p);
			s32 w1 = edgeFunction(v2, v0, p);
			s32 w2 = edgeFunction(v0, v1, p);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0){
				// Interpolate colours
				float b0 = (float)w0 / area;
				float b1 = (float)w1 / area;
				float b2 = (float)w2 / area;
				const u8 r = b0 * v0.r + b1 * v1.r + b2 * v2.r;
				const u8 g = b0 * v0.g + b1 * v1.g + b2 * v2.g;
				const u8 b = b0 * v0.b + b1 * v1.b + b2 * v2.b;
				Vertex col = { .r = r, .g = g, .b = b };
				writePixel(p.x, p.y, col.getBGR555());
			}
		}
	}
}

void GPUSoftware::drawTriTextured(Vertex v0, Vertex v1, Vertex v2, u16 clut, u16 texpage) {
	auto xMin = std::min({ v0.x, v1.x, v2.x });
	auto yMin = std::min({ v0.y, v1.y, v2.y });
	auto xMax = std::max({ v0.x, v1.x, v2.x });
	auto yMax = std::max({ v0.y, v1.y, v2.y });

	if (edgeFunction(v0, v1, v2) < 0) {
		std::swap(v1, v2);
	}

	Vertex p;

	const auto area = edgeFunction(v0, v1, v2);

	for (p.y = yMin; p.y < yMax; p.y++) {
		for (p.x = xMin; p.x < xMax; p.x++) {
			s32 w0 = edgeFunction(v1, v2, p);
			s32 w1 = edgeFunction(v2, v0, p);
			s32 w2 = edgeFunction(v0, v1, p);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				// Interpolate UV
				float b0 = (float)w0 / area;
				float b1 = (float)w1 / area;
				float b2 = (float)w2 / area;
				const u32 u = b0 * v0.u + b1 * v1.u + b2 * v2.u;
				const u32 v = b0 * v0.v + b1 * v1.v + b2 * v2.v;
				
				// Fetch texel
				const u16 texpageX = (texpage & 0xf) * 64;
				const u16 texpageY = ((texpage >> 4) & 1) * 256;
				const auto colDepth = (texpage >> 7) & 3;
				const u16 clutX = (clut & 0x3f) * 16;
				const u16 clutY = clut >> 6;
				
				u16 col = 0;
				switch (colDepth) {
				case (u32)ColDepth::Depth4: {
					col = fetch4bpp(u, v, texpageX, texpageY, clutX, clutY);
					break;
				}
				case (u32)ColDepth::Depth8: {
					col = 0xff00;
				}
				default:
					Helpers::panic("[GPU Software] Unimplemented col depth %d\n", colDepth);
				}

				if (col == 0) continue;
				writePixel(p.x, p.y, col);
			}
		}
	}
}

void GPUSoftware::drawRectUntextured(Vertex v, u16 width, u16 height) {
	for (s16 y = v.y; y < v.y + height; y++) {
		for (s16 x = v.x; x < v.x + width; x++) {
			writePixel(x, y, v.getBGR555());
		}
	}
}