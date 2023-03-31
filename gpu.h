#pragma once
#define NOMINMAX
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cstdarg>
#include <vector>
#include "glad/glad.h"

class gpu
{

public:
	// VAO & VBO for untextured polygons
	GLuint VAO = 0;
	GLuint VBO = 0;
	// VAO & VBO for textured polygons
	GLuint TextureVAO;
	GLuint TextureVBO;

	GLuint FBO = 0;
	GLuint tempFBO = 0;
	GLuint VramTexture = 0;
	GLuint SampleVramTexture = 0;
	void SyncVRAM();
	GLint oldFBO = 0;
	unsigned int id = 0;
	const GLchar* VertexShaderSource =
		R"(
		#version 330 core
		layout (location = 0) in ivec3 pos;
		uniform vec3 offset;
		layout (location = 1) in vec3 colour;
		layout (location = 2) in uint texpage;
		layout (location = 3) in uint clut;
		layout (location = 4) in vec2 texture_uv;
		layout (location = 5) in uint texture_enable;
		out vec4 frag_colour;
		void main() {
			gl_Position = vec4(float(pos.x + offset.x + 0.5) / 512 - 1, -(1 - float(pos.y + offset.y + 0.5) / 256), 0.0, 1.0);
			frag_colour = vec4(float(colour.r) / 255, float(colour.g) / 255, float(colour.b) / 255, 1.f);
		}
		)";
	const GLchar* FragmentShaderSource =
		R"(
		#version 330 core
		in vec4 frag_colour;
		out vec4 final_colour;
		void main() {
			final_colour = frag_colour;
		} 
		)";
	const GLchar* TextureVertexShaderSource =
		R"(
		#version 330 core
		layout (location = 0) in vec3 pos;
		uniform vec3 offset;
		layout (location = 1) in vec3 Colour;
		layout (location = 2) in vec2 aTexCoord;
		layout (location = 3) in vec2 aTexpageCoords;
		layout (location = 4) in vec2 aClut;
		out vec3 VertexColour;
		out vec2 TexCoord;
		flat out vec2 texpageCoords;
		flat out vec2 clut;
		uniform int colourDepth;
		void main()
		{
			gl_Position = vec4(float(pos.x + offset.x) / 512 - 1, -(1 - float(pos.y + offset.y) / 256), 0.0, 1.0);
			VertexColour = Colour;
			TexCoord = aTexCoord;
			texpageCoords = aTexpageCoords;
			clut = aClut;
		}
		)";
	const GLchar* TextureFragmentShaderSource =
		R"(
		#version 430 core
		out vec4 FragColor;
		in vec3 VertexColour;
		in vec2 TexCoord;
		flat in vec2 texpageCoords;
		flat in vec2 clut;

		uniform sampler2D vram;
		uniform int colourDepth;
		uniform ivec4 texWindow;
		int floatToU5(float f) {
			return int(floor(f * 31.0 + 0.5));
		}
		int sample16(ivec2 coords) {
			vec4 colour = texelFetch(vram, coords * 6, 0);
			int r = floatToU5(colour.r);
			int g = floatToU5(colour.g);
			int b = floatToU5(colour.b);
			int msb = int(ceil(colour.a)) << 15;
			return r | (g << 5) | (b << 10) | msb;
		}
		vec4 fetchTexel4Bit(ivec2 coords) {
			int texel = sample16(ivec2(coords.x / 4, coords.y) + ivec2(texpageCoords));
			int idx = (texel >> ((coords.x % 4) * 4)) & 0xf;
			return texelFetch(vram, ivec2(clut.x + idx, clut.y) * 6, 0);
		}
		vec4 fetchTexel8Bit(ivec2 coords) {
			int texel = sample16(ivec2(coords.x / 2, coords.y) + ivec2(texpageCoords));
			int idx = (texel >> ((coords.x % 2) * 8)) & 0xff;
			return texelFetch(vram, ivec2(clut.x + idx, clut.y) * 6, 0);
		}
		void main()
		{
			ivec2 TexCoord_ = ivec2(floor(TexCoord)) & ivec2(0xff);
			ivec2 UV = (ivec2(TexCoord_) & texWindow.xy) | texWindow.zw;
			vec4 colour;
			if(colourDepth == 0) {
				colour = fetchTexel4Bit(UV);
			}
			else if (colourDepth == 1) {
				colour = fetchTexel8Bit(UV);
			} 
			else if (colourDepth == 2) {
				vec2 TexCoords = vec2(float(UV.x + texpageCoords.x) / 1024.f - 1, -(1 - float(UV.y + texpageCoords.y) / 512.f));
				colour = texture(vram, TexCoords);
			} else colour = vec4(1.f, 0.f, 0.f, 1.f);
			if(colour.rgb == vec3(0.f, 0.f, 0.f)) discard;
			colour = (colour * vec4(VertexColour.rgb, 1.f)) / 128.f;
			colour.a = 1.f;
			FragColor = colour;
		}
		)";
	unsigned int VertexShader = 0;
	unsigned int FragmentShader = 0;
	unsigned int ShaderProgram = 0;
	unsigned int TextureShaderProgram = 0;
	unsigned int colourDepthUniform = 0;

	struct point {		// vertex struct
		int16_t x = 0, y = 0;	// coordinates
		uint32_t c = 0;		// BGR colour 
		uint8_t r = c & 0xff;
		uint8_t g = (c >> 8) & 0xff;
		uint8_t b = (c >> 16) & 0xff;
	};

	void putpixel(point v1, uint32_t colour);

public:
	gpu();
	void InitGL();

	// Upscaling
	int scaling_factor = 6;

	void ClearScreen();
	void SetOpenGLState();
	void SwitchToTextured();
	void SwitchToUntextured();
	bool DrawingTextured = false;
	uint16_t* vram = new uint16_t[1024 * 512];
	//std::vector<uint32_t> WriteBuffer;
	uint32_t* WriteBuffer = new uint32_t[(1024 * 512)];
	uint32_t* ReadBuffer = new uint32_t[(1024 * 512)];
	int WriteBufferCnt = 0;
	int ReadBufferCnt = 0;
	uint32_t* vram8 = new uint32_t[1024 * 512];
	uint32_t* vram4 = new uint32_t[1024 * 512];
	uint32_t* vram_rgb = new uint32_t[1024 * 512];

	int xpos = 0;
	int ypos = 0;	// Used to keep track of memory transfers
	uint32_t fifo[12] = {};
	int cmd_length = 0;
	int cmd_left = 0;

	int gp0_mode = 0;
	void execute_gp0(uint32_t command);
	void execute_gp1(uint32_t command);

	uint32_t drawing_topleft_x = 0;
	uint32_t drawing_topleft_y = 0;
	uint32_t drawing_bottomright_x = 0;
	uint32_t drawing_bottomright_y = 0;

	int32_t xoffset = 0;
	int32_t yoffset = 0;

	struct {
		struct {
			uint16_t x;
			uint16_t y;
		} origin;
		uint16_t width;
		uint16_t height;
	} display_area;
	int width_divisor = 0;
	uint16_t x1 = 0;
	uint16_t x2 = 0;
	uint16_t y1 = 0;
	uint16_t y2 = 0;
	void update_hres();
	void update_vres();
	int frame_counter = 0;

	// GPUSTAT
	uint8_t page_base_x = 0;
	uint8_t page_base_y = 0;
	uint8_t semi_transparency = 0;
	uint8_t texture_depth = 0;
	uint8_t dithering = 0;
	bool allow_display_drawing = false;
	bool mask_bit = false;
	bool disallow_masked_pixels_drawing = false;
	bool interlacing = false;
	bool texture_disable = false;
	uint8_t hres2 = 0;
	uint8_t hres1 = 0;
	uint8_t vres = 0;
	uint8_t video_mode = 0;
	uint8_t dma_direction = 0;
	uint16_t texpage_raw = 0;
	bool interlace = false;

	uint32_t get_status();

public:	// commands
	enum {
		MONOCHROME,
		GOURAUD
	};

	enum {
		SOLID,
		SEMI_TRANSPARENT
	};

	void draw_untextured_tri(int shading, int transparency);
	void draw_untextured_quad(int shading, int transparency);

	void texture_blending_three_point_opaque_polygon();
	void texture_three_point_opaque_polygon();
	void monochrome_line_opaque();
	void shaded_line_semi_transparent();
	void monochrome_polyline_opaque();
	void texture_blending_four_point_opaque_polygon();
	void texture_four_point_opaque_polygon();
	void texture_four_point_semi_transparent_polygon();
	void shaded_texture_blending_three_point_opaque_polygon();
	void shaded_texture_blending_textured_four_point_opaque_polygon();
	void shaded_texture_blending_textured_four_point_semi_transparent_polygon();
	void texture_blending_four_point_polygon_semi_transparent();
	void monochrome_rectangle_variable_size_opaque();
	void monochrome_rectangle_variable_size_semi_transparent();
	void texture_blending_rectangle_variable_size_opaque();
	void texture_rectangle_variable_size_opaque();
	void texture_blending_rectangle_variable_size_semi_transparent();
	void textured_rectangle_variable_size_semi_transparent();
	void texture_blending_rectangle_8x8_opaque();
	void texture_rectangle_8x8_opaque();
	void texture_blending_rectangle_16x16_opaque();
	void texture_rectangle_16x16_opaque();
	void texture_rectangle_16x16_semi_transparent();
	void monochrome_rectangle_dot_opaque();
	void monochrome_rectangle_8x8_opaque();
	void monochrome_rectangle_16x16_opaque();
	void fill_rectangle();
	void vram_to_vram();
	void cpu_to_vram();
	void vram_to_cpu();
public:
	bool debug = true;
	void debug_printf(const char* fmt, ...);


};
