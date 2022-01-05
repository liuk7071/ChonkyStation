#pragma once
#define NOMINMAX
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cstdarg>
#include "glad/glad.h"

class gpu
{

public:		// trongle stuff
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	GLuint FBO = 0; 
	GLuint VramTexture = 0;
	GLuint VramTexture8 = 0;
	GLuint VramTexture4 = 0;
	GLint oldFBO = 0;
	unsigned int id = 0;
	unsigned int VertexShader = 0;
	unsigned int FragmentShader = 0;
	unsigned int ShaderProgram = 0;
	unsigned int TextureShaderProgram = 0;
	unsigned int colourDepthUniform = 0;
	
	struct point {		// vertex struct
		uint16_t x = 0, y = 0;	// coordinates
		uint32_t c = 0;		// BGR colour
		uint8_t r = c & 0xff;
		uint8_t g = (c >> 8) & 0xff;
		uint8_t b = (c >> 16) & 0xff;
	};

	void putpixel(point v1, uint32_t colour);

public:
	gpu();
	void InitGL();
	uint16_t* vram = new uint16_t[1024 * 512];
	uint32_t* vram8 = new uint32_t[1024 * 512];
	uint32_t* vram4 = new uint32_t[1024 * 512];
	uint32_t* vram_rgb = new uint32_t[1024 * 512];

	int xpos = 0;
	int ypos = 0;	// Used to keep track of memory transfers
	uint32_t* pixels;
	uint32_t fifo[12] = {};
	int cmd_length = 0;
	int cmd_left = 0;

	int gp0_mode = 0;
	void execute_gp0(uint32_t command);
	void execute_gp1(uint32_t command);
	

	// GPUSTAT
	uint8_t page_base_x = 0;
	uint8_t page_base_y = 0;
	uint8_t semi_transparency = 0;
	uint8_t texture_depth = 0;
	uint8_t dithering = 0;
	bool allow_display_drawing = false;
	bool mask_bit = false;
	bool disallow_masked_pixels_drawing = false;
	uint8_t interlace_field = 0;
	bool texture_disable = false;
	uint8_t hres2 = 0;
	uint8_t hres1 = 0;
	uint8_t vres = 0;
	uint8_t dma_direction = 0;

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
	bool debug = false;
	void debug_printf(const char* fmt, ...);
	
	
};

