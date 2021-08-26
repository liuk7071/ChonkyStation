#pragma once
#define NOMINMAX
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cstdarg>
#include "Rasterizer.h"
#include "SDL.h"
#include "glad/glad.h"

class gpu
{

public:		// trongle stuff
	unsigned int VBO, VAO;
	GLuint FBO; // it was int before 
	unsigned int id;
	unsigned int VertexShader;
	unsigned int FragmentShader;
	unsigned int ShaderProgram;
	
	struct point {		// vertex struct
		uint16_t x, y;	// coordinates
		uint32_t c;		// BGR colour
		uint8_t r = c & 0xff;
		uint8_t g = (c >> 8) & 0xff;
		uint8_t b = (c >> 16) & 0xff;
	};
	point calc_tex_coords(int tx, int ty, int x, int y, int bpp);
	Rasterizer rast;

	void putpixel(point v1, uint32_t colour);
	void quad(point v1, point v2, point v3, point v4, uint32_t colour);

public:
	gpu();
	void InitGL();

	uint16_t vram_read(int x, int y);
	int xpos = 0;
	int ypos = 0;	// Used to keep track of memory transfers
	uint32_t* pixels;
	uint32_t fifo[12];
	int cmd_length = 0;
	int cmd_left = 0;

	int gp0_mode = 0;
	void execute_gp0(uint32_t command);
	void execute_gp1(uint32_t command);
	

	// GPUSTAT
	uint8_t page_base_x;
	uint8_t page_base_y;
	uint8_t semi_transparency;
	uint8_t texture_depth;
	uint8_t dithering;
	bool allow_display_drawing;
	bool mask_bit;
	bool disallow_masked_pixels_drawing;
	uint8_t interlace_field;
	bool texture_disable;
	uint8_t hres2;
	uint8_t hres1;
	uint8_t vres;
	uint8_t dma_direction;

	uint32_t get_status();

public:	// commands
	void monochrome_line_opaque();
	void monochrome_four_point_opaque_polygon();
	void monochrome_four_point_semi_transparent_polygon();
	void monochrome_three_point_opaque_polygon();
	void monochrome_three_point_semi_transparent_polygon();
	void texture_blending_four_point_opaque_polygon();
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
	
	
};

