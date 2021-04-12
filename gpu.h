#pragma once
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cstdarg>
#include "memory.h"
class gpu
{

public:		// rasterization stuff
	struct point {		// vertex struct
		uint16_t x, y;	// coordinates
		uint32_t c;		// BGR colour
		uint8_t r = c & 0xff;
		uint8_t g = (c >> 8) & 0xff;
		uint8_t b = (c >> 16) & 0xff;
	};


	void putpixel(point v1, uint32_t colour);
	void horizontal_line(point v1, point v2, uint32_t colour);
	void bottom_flat_triangle(point v1, point v2, point v3, uint32_t colour);
	void top_flat_triangle(point v1, point v2, point v3, uint32_t colour);
	void triangle(point v1, point v2, point v3, uint32_t colour);
	void quad(point v1, point v2, point v3, point v4, uint32_t colour);

public:
	gpu();
	uint8_t vram[512][2048];
	uint32_t pixels[480][640];
	uint32_t fifo[12];
	int cmd_length = 0;
	int cmd_left = 0;

	int gp0_mode = 0;
	void execute_gp0(uint32_t command);
	void execute_gp1(uint32_t command);
	void connectMem(memory* memory);

	// GPUSTAT
	uint8_t page_base_x;
	uint8_t page_base_y;
	uint8_t semi_transparency;
	uint8_t texture_page_colors;
	uint8_t dithering;
	bool allow_display_drawing;
	bool mask_bit;
	bool disallow_masked_pixels_drawing;
	uint8_t interlace_field;
	bool texture_disable;
	uint8_t hres2;
	uint8_t hres1;
	uint8_t vres;

	uint32_t get_status();

public:	// commands
	void monochrome_four_point_opaque_polygon();
	void monochrome_four_point_semi_transparent_polygon();
	void monochrome_three_point_opaque_polygon();
	void monochrome_three_point_semi_transparent_polygon();
	void shaded_three_point_opaque_polygon();
	void shaded_three_point_semi_transparent_polygon();
	void shaded_four_point_opaque_polygon();
	void shaded_four_point_semi_transparent_polygon();
	void monochrome_rectangle_dot_opaque();
	void fill_rectangle();
	void cpu_to_vram();
	void vram_to_cpu();
public:
	void write32(uint32_t addrX, uint32_t addrY, uint32_t data);
public:
	bool debug;
	void debug_printf(const char* fmt, ...);
	memory mem;
	
};

