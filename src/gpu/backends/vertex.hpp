#pragma once

#include <helpers.hpp>


// Vertex type
struct Vertex {
	s16 x, y;
	u8 r, g, b;
	u8 u, v;
	void writeBGR888(u32 bgr) {
		r = bgr & 0xff;
		g = (bgr >> 8) & 0xff;
		b = (bgr >> 16) & 0xff;
	}
	void writeBGR555(u16 bgr) {
		r = (bgr & 0x1f) << 3;
		g = ((bgr >> 5) & 0x1f) << 3;
		b = ((bgr >> 10) & 0x1f) << 3;
	}
	u32 getBGR888() {
		return r | (g << 8) | (b << 16) | (0xff << 24);
	}
	u16 getBGR555() {
		return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | (1 << 15);
	}
	u16 getRGB555() {
		return (b >> 3) | ((g >> 3) << 5) | ((r >> 3) << 10) | (1 << 15);
	}
};