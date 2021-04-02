#pragma once
#include <stdint.h>
#include <cstdarg>
#include "memory.h"
class gpu
{
public:
	gpu();
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
	void monochrome_rectangle_dot_opaque();
	void cpu_to_vram();
	void vram_to_cpu();

public:
	bool debug;
	void debug_printf(const char* fmt, ...);
	memory mem;
	
};

