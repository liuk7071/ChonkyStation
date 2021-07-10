#include "gpu.h"
#include "Bus.h"
#include <cmath>
Bus* bus;

gpu::gpu() {

	
	// Initialize pixel array
	pixels = new uint32_t [480 * 640];

	rast.SetFrameBuffer((uint32_t*)pixels, 640, 480);

	debug = false;
	point v1, v2, v3, v4;
	v1.x = 0; 
	v1.y = 0;
	v2.x = 640;
	v2.y = 0;
	v3.x = 0;
	v3.y = 480;
	v4.x = 640;
	v4.y = 480;

	//quad(v1, v2, v3, v4, 0xff00);
	//triangle(v1, v2, v3, 0xff);
}

void connectBus(Bus *_bus) {
	bus = _bus;
}


void gpu::debug_printf(const char* fmt, ...) {
	if (debug) {
		std::va_list args;
		va_start(args, fmt);
		std::vprintf(fmt, args);
		va_end(args);
	}
}

uint32_t gpu::get_status() {
	uint32_t status = 0;

	status |= page_base_x << 0;
	status |= page_base_y << 3;

	return status;
}


// trongles
//static const GLchar* vertexShaderSource = 
//R"(
//#version 330 core
//in ivec3 vposition;
//in uvec3 vcolor;
//out vec3 color;
//void main() {
//    float xpos = (float(vposition.x) / 512) - 1.0;
//    float ypos = 1.0 - (float(vposition.y) / 256);
//    gl_Position.xyzw = vec4(xpos, ypos, 0.0, 1.0);
//    color = vec3(float(vcolor.r) / 255, float(vcolor.g) / 255, float(vcolor.b) / 255);
//}
//)";
//static const GLchar* fragmentShaderSource =
//R"(
//#version 330 core
//in vec3 color;
//out vec4 frag_color;
//void main() {
//    frag_color = vec4(color, 1.0);
//}
//)";

static const GLchar* vertexShaderSource =
R"(
#version 430 core
layout (location = 0) in vec2 vpos;
layout (location = 1) in vec2 tex_coord;
out vec2 tex_coords;

uniform ivec2 offset = ivec2(0);

void main()
{
	/* Add the draw offset. */  
	vec2 pos = vpos + offset;

	/* Transform from 0-640 to 0-1 range. */
	float posx = pos.x / 640 * 2 - 1;
	/* Transform from 0-480 to 0-1 range. */
        float posy = pos.y / 480 * (-2) + 1;

	/* Emit vertex. */
	gl_Position = vec4(posx, posy, 0.0, 1.0);
	tex_coords = tex_coord;
}
)";
static const GLchar* fragmentShaderSource =
R"(
#version 430 core
in vec2 tex_coords;
out vec4 frag_color;

uniform int texture_depth;

/* Used for palleted texture lookup. */ 
uniform sampler2D texture_sample4;
uniform sampler2D texture_sample8;
uniform sampler2D texture_sample16;

uniform int clut4[16];
uniform int clut8[128];

vec4 split_colors(int data)
{
    vec4 color;
    color.r = (data << 3) & 0xf8;
    color.g = (data >> 2) & 0xf8;
    color.b = (data >> 7) & 0xf8;
    color.a = 255.0f;

    return color;
}

vec4 sample_texel()
{
    if (texture_depth == 4) {
        vec4 index = texture2D(texture_sample4, tex_coords);
        int texel = clut4[int(index.r * 255)];

        return split_colors(texel) / vec4(255.0f);
    }
    else if (texture_depth == 8) {
        vec4 index = texture2D(texture_sample8, tex_coords);
        int texel = clut8[int(index.r * 255)];

        return split_colors(texel) / vec4(255.0f);  
    }
    else {
        vec4 texel = texture2D(texture_sample16, tex_coords);
		int r = int(texel.r * 255);
        return split_colors(r) / vec4(255.0f);
    }
}

void main()
{
	frag_color = sample_texel();
}
)";

void gpu::GetGlContext(SDL_GLContext* glc) {
	GlContext = &glc;
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
	}

	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
	}
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}
	//GLuint vao, vbo;
	//glGenVertexArrays(1, &vao);
	//glGenBuffers(1, &vbo);
	//glBindVertexArray(vao);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//auto buffer_size = sizeof(uint16_t) * (64 * 1024);
	//glBufferStorage(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
	//glMapBufferRange(GL_ARRAY_BUFFER, 0, buffer_size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

	uint32_t buffer_mode = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
	glGenBuffers(1, &pbo16);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);

	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512, nullptr, buffer_mode);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &texture16);
	glBindTexture(GL_TEXTURE_2D, texture16);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, 1024, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, texture16);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);

	ptr16 = (uint16_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512, buffer_mode);

	glGenBuffers(1, &pbo4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo4);

	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512 * 4, nullptr, buffer_mode);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &texture4);
	glBindTexture(GL_TEXTURE_2D, texture4);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4096, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, texture4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo4);

	ptr4 = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512 * 4, buffer_mode);

	glGenBuffers(1, &pbo8);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo8);

	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512 * 2, nullptr, buffer_mode);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &texture8);
	glBindTexture(GL_TEXTURE_2D, texture8);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 2048, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, texture8);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo8);

	ptr8 = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512 * 2, buffer_mode);

	
	
}
void gpu::upload_to_gpu()
{
	glBindTexture(GL_TEXTURE_2D, texture16);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RED, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, texture4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo4);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4096, 512, GL_RED, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, texture8);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo8);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2048, 512, GL_RED, GL_UNSIGNED_BYTE, 0);
}
uint16_t gpu::readv(uint32_t x, uint32_t y)
{
	int index = (y * 1024) + x;
	return ptr16[index];
}

void gpu::writev(uint32_t x, uint32_t y, uint16_t data)
{
	int index = (y * 1024) + x;

	ptr16[index] = data;

	ptr8[index * 2 + 0] = (uint8_t)data;
	ptr8[index * 2 + 1] = (uint8_t)(data >> 8);

	ptr4[index * 4 + 0] = (uint8_t)data & 0xf;
	ptr4[index * 4 + 1] = (uint8_t)(data >> 4) & 0xf;
	ptr4[index * 4 + 2] = (uint8_t)(data >> 8) & 0xf;
	ptr4[index * 4 + 3] = (uint8_t)(data >> 12) & 0xf;
}

gpu::point calc_tex_coords(int tx, int ty, int x, int y, int bpp) {
	gpu::point p;
	double r = 16 / bpp;
	double xc = (tx * r + x) / (1024.0 * r);
	double yc = (ty + y) / 512.0;
	p.x = xc;
	p.y = yc;
	return p;
}

void gpu::putpixel(point v1, uint32_t colour) {
	//Color c1(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0);
	//if (v1.x >= 1024 || v1.y >= 512)
	//	return;
	//
	//rast.vram[v1.y * 1024 + v1.x] = c1.ToUInt32();

	rast.SetPixel(v1.x, v1.y, colour);
}

void gpu::quad(point v1, point v2, point v3, point v4, uint32_t colour) {
	Color c1(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16);
	Color c2(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16);
	Color c3(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16);;

	

	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	rast.DrawTriangle(c1, v2.x, v2.y, c2, v3.x, v3.y, c3, v4.x, v4.y);
	//triangle(v1, v2, v3, colour);
	//triangle(v2, v3, v4, colour);
}

// Textures
uint16_t gpu::vram_read(int x, int y) {
	return rast.vram[y * 1024 + x];
}

void gpu::execute_gp0(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;
	if (cmd_left == 0) {
		cmd_length = 0;
		switch (instr) {
		case(0):	// nop
			debug_printf("[GP0] NOP (0x%x)\n", command);
			break;
		case(0x01): {	// Clear Cache
			debug_printf("[GP0] Clear Cache\n");
			break;
		}
		case(0x02): {	// Fill rectangle in VRAM
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case(0x20): { // Monochrome three - point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case(0x22): { // Monochrome three-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case(0x28): { // Monochrome four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 4;
			break;
		}
		case(0x2A): { // Monochrome four-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 4;
			break;
		}
		case(0x2C): { // Textured four-point polygon, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 8;
			break;
		}
		case(0x30): {	// Shaded three-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 5;
			break;
		}
		case(0x32): { // Shaded three-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 5;
			break;
		}
		case(0x38): { // Shaded four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 7;
			break;
		}
		case(0x3A): { // Shaded four-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 7;
			break;
		}
		case(0x68): {	// 1x1 Opaque Monochrome Rectangle
			fifo[0] = command;
			cmd_length++;
			cmd_left = 1;
			break;
		}
		case(0xA0): {	// Copy rectangle CPU to VRAM
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case(0xC0): {	// Copy rectangle VRAM to CPU
			// unimplemented
			debug_printf("[GP0] Copy Rectangle (VRAM to CPU)\n");
			break;
		}
		case(0xE1): {	// Draw Mode Setting
			debug_printf("[GP0] Draw Mode Setting\n");
			page_base_x = command & 0xf;
			page_base_y = (command >> 4) & 1;
			semi_transparency = (command >> 5) & 3;
			texture_depth = (command >> 7) & 3;
			dithering = ((command >> 9) & 1) != 0;
			allow_display_drawing = ((command >> 10) & 1) != 0;
			// rectangle x flip
			// rectangle y flip
			break;
		}
		case(0xE2): {	// Set Texture Window
			// unimplemented
			debug_printf("[GP0] Set Texture Window\n");
			break;
		}
		case(0xE3): {	// Set Drawing Area top left
			// unimplemented
			debug_printf("[GP0] Set Drawing Area top left\n");
			break;
		}
		case(0xE4): {	// Set Drawing Area bottom right
			// unimplemented
			debug_printf("[GP0] Set Drawing Area bottom right\n");
			break;
		}
		case(0xE5): {	// Set Drawing Area Offset
			// unimplemented
			debug_printf("[GP0] Set Drawing Area Offset\n");
			break;
		}
		case(0xE6): {	// Set Mask Bit Setting
			debug_printf("[GP0] Set Mask Bit Setting\n");
			mask_bit = (command & 1) != 0;
			disallow_masked_pixels_drawing = (command & 2) != 0;
			break;
		}
		default:
			printf("\n[GP0] Unknown GP0 command: 0x%x (0x%x)\n", instr, command);
			//exit(0);
		}
	}
	else {
		cmd_left--;
		switch (gp0_mode) {
		case 0: {	// command mode
			fifo[cmd_length++] = command;

			if (cmd_left == 0) {	// all the parameters are in, run command

				switch ((fifo[0] >> 24) & 0xff) {

				case(0x02): gpu::fill_rectangle(); break;
				case(0x20): gpu::monochrome_three_point_opaque_polygon(); break;
				case(0x22): gpu::monochrome_three_point_semi_transparent_polygon(); break;
				case(0x28): gpu::monochrome_four_point_opaque_polygon(); break;
				case(0x2A): gpu::monochrome_four_point_semi_transparent_polygon(); break;
				case(0x2C): gpu::texture_blending_four_point_opaque_polygon(); break;
				case(0x30): gpu::shaded_three_point_opaque_polygon(); break;
				case(0x32): gpu::shaded_three_point_semi_transparent_polygon(); break;
				case(0x38): gpu::shaded_four_point_opaque_polygon(); break;
				case(0x3A): gpu::shaded_four_point_semi_transparent_polygon(); break;
				case(0x68): gpu::monochrome_rectangle_dot_opaque(); break;
				case(0xA0): gpu::cpu_to_vram(); break;
				
				}
			}
			break;
		}
		case 1: {	// load mode
			debug_printf("[CPU to VRAM transfer] Data: 0x%x\n", command);
			uint32_t resolution = fifo[2];
			uint32_t coords = fifo[1];
			auto width = resolution & 0xffff;
			auto height = resolution >> 16;
			if (width == 0) width = 1024;
			if (height == 0) height = 512;
			auto x = coords & 0xffff;
			auto y = coords >> 16;
			
			xpos %= 1024;
			ypos %= 512;
			
			uint32_t c = command & 0xffff;
			uint32_t a = (c >> 15) & 1;
			uint32_t b = (c >> 10) & 0b11111;
			uint32_t g = (c >> 5) & 0b11111;
			uint32_t r = c & 0b11111;
			uint32_t rgba = ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
			rast.vram[(y + ypos) * 1024 + (x+xpos)] = command & 0xffff;
			rast.vram_rgb[(y + ypos) * 1024 + (x + xpos)] = rgba;
			//writev(x + xpos, y + ypos, command & 0xffff);
			xpos++;

			if (xpos == width) {
				xpos = 0;
				ypos++;

				if (ypos == height) {
					gp0_mode = 0;
					//upload_to_gpu();
					break;
				}
			}

			c = command >> 16;
			a = (c >> 15) & 1;
			b = (c >> 10) & 0b11111;
			g = (c >> 5) & 0b11111;
			r = c & 0b11111;
			rgba = ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
			rast.vram[(y+ypos) * 1024 + (x + xpos)] = command >> 16;
			rast.vram_rgb[(y + ypos) * 1024 + (x + xpos)] = rgba;
			//writev(x + xpos, y + ypos, command >> 16);
			xpos++;

			if (xpos == width) {
				xpos = 0;
				ypos++;

				if (ypos == height) {
					gp0_mode = 0;
					//upload_to_gpu();
					break;
				}
			}

		}
		}
	}
}

void gpu::execute_gp1(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;

	switch (instr) {
	case(0x0):	// reset gpu
		debug_printf("[GP1] Reset Gpu\n");
		break;
	case(0x1): // reset command buffer
		debug_printf("[GP1] Reset Command Buffer\n");
		break;
	case(0x4): { // set dma direction
		debug_printf("[GP1] Set DMA Direction to %d\n", command & 0b11);
		dma_direction = command & 0b11;
		break;
	}
	default:
		debug_printf("[GP1] Unknown GP1 command: 0x%x\n", instr);
		//exit(0);
	}
}

// commands
void gpu::monochrome_four_point_opaque_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Monochrome four-point polygon, opaque (colour: 0x%x)\n", colour);
	point v1, v2, v3, v4;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;
	v3.x = fifo[3] & 0xffff;
	v3.y = fifo[3] >> 16;
	v4.x = fifo[4] & 0xffff;
	v4.y = fifo[4] >> 16;
	quad(v1, v2, v3, v4, colour);
	return;
}
void gpu::texture_blending_four_point_opaque_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, opaque, texture blending (colour: 0x%x)\n", colour);
	point v1, v2, v3, v4, t1, t2, t3, t4;
	Rasterizer::point page, clut;
	page.x = page_base_x;
	page.y = page_base_y;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	clut.x = fifo[2] & 0b111111;
	clut.y = (fifo[2] & 0b111111111) >> 5;
	page.x = fifo[5] & 0b111111;
	page.y = (fifo[5] & 0b111111111) >> 5;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;

	t1.x = fifo[2] & 0b11;
	t1.y = (fifo[2] & 0b1100) >> 2;
	t2.x = fifo[4] & 0b11;
	t2.y = (fifo[4] & 0b1100) >> 2;
	t3.x = fifo[6] & 0b11;
	t3.y = (fifo[6] & 0b1100) >> 2;
	t4.x = fifo[8] & 0b11;
	t4.y = (fifo[8] & 0b1100) >> 2;
	//rast.textured = true;
	rast.page = page;
	rast.clut = clut;
	if (texture_depth == 0) rast.depth = Rasterizer::Depth::BITS4;
	if (texture_depth == 1) rast.depth = Rasterizer::Depth::BITS8;
	if (texture_depth == 2) rast.depth = Rasterizer::Depth::BITS16;
	quad(v1, v2, v3, v4, 0xff);
	//rast.textured = false;
	return;
}
void gpu::monochrome_four_point_semi_transparent_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Monochrome four-point polygon, semi-transparent (colour: 0x%x)\n", colour);
	point v1, v2, v3, v4;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;
	v3.x = fifo[3] & 0xffff;
	v3.y = fifo[3] >> 16;
	v4.x = fifo[4] & 0xffff;
	v4.y = fifo[4] >> 16;
	quad(v1, v2, v3, v4, colour);
	return;
}
void gpu::monochrome_three_point_opaque_polygon() {
	debug_printf("[GP0] Monochrome three-point polygon, opaque\n");
	point v1, v2, v3;
	uint32_t colour = fifo[0] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;
	v3.x = fifo[3] & 0xffff;
	v3.y = fifo[3] >> 16;

	Color c1(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0);
	Color c2(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0);
	Color c3(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0);



	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	return;
}
void gpu::monochrome_three_point_semi_transparent_polygon() {
	debug_printf("[GP0] Monochrome three-point polygon, semi-transparent\n");
	point v1, v2, v3;
	uint32_t colour = fifo[0] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;
	v3.x = fifo[3] & 0xffff;
	v3.y = fifo[3] >> 16;

	Color c1(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0x7f);
	Color c2(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0x7f);
	Color c3(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0x7f);



	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	return;
}
void gpu::shaded_three_point_opaque_polygon() {
	debug_printf("[GP0] Shaded three-point polygon, opaque\n");
	point v1, v2, v3;
	v1.c = fifo[0] & 0xffffff;
	v2.c = fifo[2] & 0xffffff;
	v3.c = fifo[4] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;

	Color c1(v1.c & 0xff, (v1.c & 0xff00) >> 8, (v1.c & 0xff0000) >> 16);
	Color c2(v2.c & 0xff, (v2.c & 0xff00) >> 8, (v2.c & 0xff0000) >> 16);
	Color c3(v3.c & 0xff, (v3.c & 0xff00) >> 8, (v3.c & 0xff0000) >> 16);



	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	return;
}
void gpu::shaded_three_point_semi_transparent_polygon() {
	debug_printf("[GP0] Shaded three-point polygon, semi-transparent\n");
	point v1, v2, v3;
	v1.c = fifo[0] & 0xffffff;
	v2.c = fifo[2] & 0xffffff;
	v3.c = fifo[4] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;

	Color c1(v1.c & 0xff, (v1.c & 0xff00) >> 8, (v1.c & 0xff0000) >> 16, 0x7f);
	Color c2(v2.c & 0xff, (v2.c & 0xff00) >> 8, (v2.c & 0xff0000) >> 16, 0x7f);
	Color c3(v3.c & 0xff, (v3.c & 0xff00) >> 8, (v3.c & 0xff0000) >> 16, 0x7f);



	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	return;
}
void gpu::shaded_four_point_opaque_polygon() {
	point v1, v2, v3, v4;
	v1.c = fifo[0] & 0xffffff;
	v2.c = fifo[2] & 0xffffff;
	v3.c = fifo[4] & 0xffffff;
	v4.c = fifo[6] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;
	debug_printf("[GP0] Shaded four-point polygon, opaque\n");

	Color c1(v1.c & 0xff, (v1.c & 0xff00) >> 8, (v1.c & 0xff0000) >> 16);
	Color c2(v2.c & 0xff, (v2.c & 0xff00) >> 8, (v2.c & 0xff0000) >> 16);
	Color c3(v3.c & 0xff, (v3.c & 0xff00) >> 8, (v3.c & 0xff0000) >> 16);
	Color c4(v4.c & 0xff, (v4.c & 0xff00) >> 8, (v4.c & 0xff0000) >> 16);

	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	rast.DrawTriangle(c2, v2.x, v2.y, c3, v3.x, v3.y, c4, v4.x, v4.y);
	return;
}
void gpu::shaded_four_point_semi_transparent_polygon() {
	point v1, v2, v3, v4;
	v1.c = fifo[0] & 0xffffff;
	v2.c = fifo[2] & 0xffffff;
	v3.c = fifo[4] & 0xffffff;
	v4.c = fifo[6] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;
	debug_printf("[GP0] Shaded four-point polygon, semi-transparent\n");
	Color c1(v1.c & 0xff, (v1.c & 0xff00) >> 8, (v1.c & 0xff0000) >> 16, 0x7f);
	Color c2(v2.c & 0xff, (v2.c & 0xff00) >> 8, (v2.c & 0xff0000) >> 16, 0x7f);
	Color c3(v3.c & 0xff, (v3.c & 0xff00) >> 8, (v3.c & 0xff0000) >> 16, 0x7f);
	Color c4(v4.c & 0xff, (v4.c & 0xff00) >> 8, (v4.c & 0xff0000) >> 16, 0x7f);

	rast.DrawTriangle(c1, v1.x, v1.y, c2, v2.x, v2.y, c3, v3.x, v3.y);
	rast.DrawTriangle(c2, v2.x, v2.y, c3, v3.x, v3.y, c4, v4.x, v4.y);
	return;
}
void gpu::monochrome_rectangle_dot_opaque() {
	uint16_t x = fifo[1] & 0xffff;
	uint16_t y = fifo[1] >> 16;
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] 1x1 Draw Opaque Monochrome Rectangle (coords: %d;%d colour: 0x%x)\n", x, y, colour);
	if (x >= 640 || y >= 480)
		return;
	Color c1(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0);
	rast.vram[y * 1024 + x] = c1.ToUInt32();
	return;
}
void gpu::fill_rectangle() {
	debug_printf("[GP0] Fill Rectangle\n");
	return;
}
void gpu::cpu_to_vram() {
	debug_printf("[GP0] Copy Rectangle (CPU to VRAM)\n");
	uint32_t resolution = fifo[2];
	uint32_t coords = fifo[1];
	auto width = resolution & 0xffff;
	auto height = resolution >> 16;
	xpos = 0;
	ypos = 0;
	uint32_t size = width * height;
	size += 1;
	size &= ~1;
	cmd_left = size / 2;
	gp0_mode = 1;
	return;
}

void gpu::vram_to_cpu() {
	uint32_t resolution = fifo[2];
	auto width = resolution & 0xffff;
	auto height = resolution >> 16;
	return;
}

