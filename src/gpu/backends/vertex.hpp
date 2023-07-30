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
	u32 getBGR888() {
		return r | (g << 8) | (b << 16) | (0xff << 24);
	}
	u16 getBGR555() {
		return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | (1 << 15);
	}
};