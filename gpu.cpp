#include "gpu.h"
#include "Bus.h"
#include <cmath>
Bus* bus;

gpu::gpu() {
	// Initialize pixel array
	pixels = new uint32_t [480 * 640];
	debug = false;
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

static const GLchar* VertexShaderSource =
R"(
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 colour;
layout (location = 2) in uint texpage;
layout (location = 3) in uint clut;
layout (location = 4) in vec2 texture_uv;
layout (location = 5) in uint texture_enable;
out vec3 frag_colour;
out vec2 frag_texture_uv;
flat out uint frag_texture_enable;
flat out uvec2 texture_base;
out uint texture_mode;
flat out uint frag_clut;

void main() {
	gl_Position = vec4(float(pos.x) / 320 - 1, -(1 - float(pos.y) / 240), 0.0, 1.0);
	frag_colour = vec3(float(colour.r) / 255, float(colour.g) / 255, float(colour.b) / 255);
	frag_texture_uv = texture_uv;
	frag_texture_enable = texture_enable;
	texture_mode = (texpage >> 7) & 0x3u;
	texture_base = uvec2((texpage & 0xfu) * 64u, ((texpage >> 4) & 1u) * 256u);
	frag_clut = clut;
}
)";
static const GLchar* TextureVertexShaderSource =
R"(
#version 130

in vec2 position;
in vec3 color;
in vec2 texture_uv;
in uint texpage;
in uint clut;

out vec3 frag_color;
out vec2 frag_texture_uv;
flat out uint frag_texture_enable;
flat out uint frag_texture_mode;
flat out uvec2 frag_texture_base;
flat out uint frag_clut;

void main()
{
  gl_Position = vec4(position.x / 320 - 1.0, 1.0 - position.y / 240, 0.0, 1.0);
  frag_color = color;
  frag_texture_uv = texture_uv;
  frag_texture_enable = (texpage & 0x8000u) >> 15;
  frag_texture_mode = (texpage >> 7) & 3u;
  frag_texture_base = uvec2((texpage & 0xfu) * 64u, ((texpage>>4) & 1u) * 256u);
  frag_clut = clut;
}
)";
static const GLchar* FragmentShaderSource =
R"(
#version 330 core
in vec3 frag_colour;
in vec2 frag_texture_uv;
flat in uint texture_enable;
flat in uvec2 texture_base;
in uint texture_mode;
flat in uint frag_clut;
uniform usampler2D vram;
out vec3 final_colour;

void main() {
	final_colour = frag_colour;
} 
)";
static const GLchar* TextureFragmentShaderSource =
R"(
#version 130

in vec3 frag_color;
in vec2 frag_texture_uv;
flat in uint frag_texture_enable;
flat in uint frag_texture_mode;
flat in uvec2 frag_texture_base;
flat in uint frag_clut;
uniform usampler2D vram_texture;

out vec3 out_color;

 uint vram_read(uint x, uint y) {
     return texelFetch(vram_texture, ivec2(int(x), int(y)), 0).r;
 }

void main() {
    // Textures enabled
    uint tex_x = frag_texture_base.x + uint(frag_texture_uv.x)/4u;
    uint tex_y = frag_texture_base.y + uint(frag_texture_uv.y);
    uint texel = vram_read(tex_x, tex_y);
    uint clut_index;
    switch(int(frag_texture_base.x + uint(frag_texture_uv.x)) % 4) {
      case 0: clut_index = (texel >> 0)  & 0xfu; break;
      case 1: clut_index = (texel >> 4)  & 0xfu; break;
      case 2: clut_index = (texel >> 8)  & 0xfu; break;
      case 3: clut_index = (texel >> 12) & 0xfu; break;
    }
    uint frag_clut_x = frag_clut & 0x3fu;
    uint frag_clut_y = frag_clut >> 6;
    uint pixel_data = vram_read(frag_clut_x*16u + clut_index, frag_clut_y);
    out_color = vec3(float((pixel_data>>0) & 0x1fu)/31, float((pixel_data>>5) & 0x1fu)/31, float((pixel_data>>10) & 0x1fu)/31);
    //if(out_color == vec3(0,0,0)) discard;
}
)";

void gpu::InitGL() {
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
	glCompileShader(VertexShader);
	int success;
	char InfoLog[512];
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
		std::cout << "Vertex shader compilation failed\n" << InfoLog << std::endl;
	}
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(FragmentShader, 512, NULL, InfoLog);
		std::cout << "Fragment shader compilation failed\n" << InfoLog << std::endl;
	}
	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ShaderProgram, 512, NULL, InfoLog);
		std::cout << "Linking shader program failed\n" << InfoLog << std::endl;
	}

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &TextureVertexShaderSource, NULL);
	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
		std::cout << "Vertex shader compilation failed\n" << InfoLog << std::endl;
	}
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &TextureFragmentShaderSource, NULL);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(FragmentShader, 512, NULL, InfoLog);
		std::cout << "Fragment shader compilation failed\n" << InfoLog << std::endl;
	}
	TextureShaderProgram = glCreateProgram();
	glAttachShader(TextureShaderProgram, VertexShader);
	glAttachShader(TextureShaderProgram, FragmentShader);
	glLinkProgram(TextureShaderProgram);
	glGetProgramiv(TextureShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(TextureShaderProgram, 512, NULL, InfoLog);
		std::cout << "Linking shader program failed\n" << InfoLog << std::endl;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glGenTextures(1, &VramTexture);
	glBindTexture(GL_TEXTURE_2D, VramTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	uint32_t buffer_mode = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
	glGenBuffers(1, &pbo16);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);

	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512, nullptr, buffer_mode);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &tex16);
	glBindTexture(GL_TEXTURE_2D, tex16);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, 1024, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, tex16);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);

	ptr16 = (uint16_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512, buffer_mode);
	glGenBuffers(1, &pbo4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo4);

	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512 * 4, nullptr, buffer_mode);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &tex4);
	glBindTexture(GL_TEXTURE_2D, tex4);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4096, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, tex4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo4);

	ptr4 = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512 * 4, buffer_mode);

	glGenBuffers(1, &pbo8);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo8);

	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, 1024 * 512 * 2, nullptr, buffer_mode);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &tex8);
	glBindTexture(GL_TEXTURE_2D, tex8);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 2048, 512, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, tex8);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo8);

	ptr8 = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024 * 512 * 2, buffer_mode);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glUseProgram(ShaderProgram);
}
void gpu::UpdateTextures() {
	glBindTexture(GL_TEXTURE_2D, tex16);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo16);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RED, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, tex4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo4);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4096, 512, GL_RED, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, tex8);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo8);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2048, 512, GL_RED, GL_UNSIGNED_BYTE, 0);
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
	// TODO: OpenGL implementation
}


// Textures
uint16_t gpu::vram_read(int x, int y) {
	//return vram[y * 1024 + x];
	int index = (y * 1024) + x;
	return ptr16[index];
}
void gpu::vram_write(int x, int y, uint16_t data) {
	int index = (y * 1024) + x;
	ptr16[index] = data;

	ptr8[index * 2 + 0] = (uint8_t)data;
	ptr8[index * 2 + 1] = (uint8_t)(data >> 8);

	ptr4[index * 4 + 0] = (uint8_t)data & 0xf;
	ptr4[index * 4 + 1] = (uint8_t)(data >> 4) & 0xf;
	ptr4[index * 4 + 2] = (uint8_t)(data >> 8) & 0xf;
	ptr4[index * 4 + 3] = (uint8_t)(data >> 12) & 0xf;
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
		case(0x40): { // Monochrome line, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
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
				case(0x40): gpu::monochrome_line_opaque();
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
			vram[(y + ypos) * 1024 + (x+xpos)] = command & 0xffff;
			vram_rgb[(y + ypos) * 1024 + (x + xpos)] = rgba;
			//vram_write(x + xpos, y + ypos, command & 0xffff);
			//vram_rgb[(y + ypos) * 1024 + (x + xpos)] = rgba;
			//writev(x + xpos, y + ypos, command & 0xffff);
			xpos++;

			if (xpos == width) {
				xpos = 0;
				ypos++;

				if (ypos == height) {
					gp0_mode = 0;
					//UpdateTextures();
					//glBindTexture(GL_TEXTURE_2D, VramTexture);
					//glTexSubImage2D(GL_TEXTURE_2D, 0, GL_R16UI, 1024, 512, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, vram);
					//glGenerateMipmap(GL_TEXTURE_2D);
					break;
				}
			}

			c = command >> 16;
			a = (c >> 15) & 1;
			b = (c >> 10) & 0b11111;
			g = (c >> 5) & 0b11111;
			r = c & 0b11111;
			rgba = ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
			vram[(y+ypos) * 1024 + (x + xpos)] = command >> 16;
			vram_rgb[(y + ypos) * 1024 + (x + xpos)] = rgba;
			//vram_write(x + xpos, y + ypos, command >> 16);
			//vram_rgb[(y + ypos) * 1024 + (x + xpos)] = rgba;
			//writev(x + xpos, y + ypos, command >> 16);
			xpos++;

			if (xpos == width) {
				xpos = 0;
				ypos++;

				if (ypos == height) {
					gp0_mode = 0;
					//UpdateTextures();
					//glBindTexture(GL_TEXTURE_2D, VramTexture);
					//glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, 1024, 512, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, vram);
					//glGenerateMipmap(GL_TEXTURE_2D);
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

	uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,
		v3.x, v3.y, 0,
	
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	
	uint32_t Vertices2[]{
		v2.x, v2.y, 0.0,
		v3.x, v3.y, 0.0,
		v4.x, v4.y, 0.0,
	
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	return;
}

void gpu::texture_blending_four_point_opaque_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, opaque, texture blending (colour: 0x%x)\n", colour);
	point v1, v2, v3, v4;
	uint16_t texpage = 0;
	uint16_t clut = 0;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	clut = fifo[2] >> 16;
	texpage = fifo[4] >> 16;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;

	uint16_t t1 = fifo[2] & 0xffff;
	uint16_t t2 = fifo[4] & 0xffff;
	uint16_t t3 = fifo[6] & 0xffff;
	uint16_t t4 = fifo[8] & 0xffff;

	//t1.x = fifo[2] & 0b11;
	//t1.y = (fifo[2] & 0b1100) >> 2;
	//t2.x = fifo[4] & 0b11;
	//t2.y = (fifo[4] & 0b1100) >> 2;
	//t3.x = fifo[6] & 0b11;
	//t3.y = (fifo[6] & 0b1100) >> 2;
	//t4.x = fifo[8] & 0b11;
	//t4.y = (fifo[8] & 0b1100) >> 2;

	/*uint32_t Vertices1[]{
		v1.x, v1.y,
		v2.x, v2.y,
		v3.x, v3.y,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),

		texpage, clut,

		(t1 >> 8), t1 & 0xff, (t2 >> 8), t2 & 0xff, (t3 >> 8), t3 & 0xff, (t4 >> 8), t4 & 0xff
	};*/
	uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,
		v3.x, v3.y, 0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};
	//glBindTexture(GL_TEXTURE_2D, VramTexture);

	/*glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 2, GL_UNSIGNED_INT, GL_FALSE, 2 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(7 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_INT, GL_FALSE, 2 * sizeof(uint32_t), (void*)(18 * sizeof(uint32_t)));
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (void*)(16 * sizeof(uint32_t)));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, (void*)(17 * sizeof(uint32_t)));
	glEnableVertexAttribArray(4);
	glUseProgram(TextureShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);*/

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	/*uint32_t Vertices2[]{
		v2.x, v2.y, 0.0,
		v3.x, v3.y, 0.0,
		v4.x, v4.y, 0.0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),

		texpage, clut,

		t1, t2, t3, t4, 1
	};*/
	uint32_t Vertices2[]{
		v2.x, v2.y, 0.0,
		v3.x, v3.y, 0.0,
		v4.x, v4.y, 0.0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};
	/*glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(uint32_t), (void*)(18 * sizeof(uint32_t)));
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)(19 * sizeof(uint32_t)));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(4, 2, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)(20 * sizeof(uint32_t)));
	glEnableVertexAttribArray(4);
	glUseProgram(TextureShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);*/

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
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
	
	// TODO: OpenGL implementation
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

	uint32_t Vertices1[]{
		v1.x, v1.y, 0.0,
		v2.x, v2.y, 0.0,
		v3.x, v3.y, 0.0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
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
	
	// TODO: OpenGL implementation
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

	uint32_t Vertices[]{
		v1.x, v1.y, 0.0, 
		v2.x, v2.y, 0.0, 
		v3.x, v3.y, 0.0, 

		(((v1.c) >> 0) & 0xff), (((v1.c) >> 8) & 0xff), (((v1.c) >> 16) & 0xff),
		(((v2.c) >> 0) & 0xff), (((v2.c) >> 8) & 0xff), (((v2.c) >> 16) & 0xff),
		(((v3.c) >> 0) & 0xff), (((v3.c) >> 8) & 0xff), (((v3.c) >> 16) & 0xff)
	};
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9*sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
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

	// TODO: OpenGL implementation
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

	uint32_t Vertices1[]{
		v1.x, v1.y, 0.0,
		v2.x, v2.y, 0.0,
		v3.x, v3.y, 0.0,
	
		(((v1.c) >> 0) & 0xff), (((v1.c) >> 8) & 0xff), (((v1.c) >> 16) & 0xff),
		(((v2.c) >> 0) & 0xff), (((v2.c) >> 8) & 0xff), (((v2.c) >> 16) & 0xff),
		(((v3.c) >> 0) & 0xff), (((v3.c) >> 8) & 0xff), (((v3.c) >> 16) & 0xff)
	};
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	uint32_t Vertices2[]{
		v2.x, v2.y, 0.0,
		v3.x, v3.y, 0.0,
		v4.x, v4.y, 0.0,

		(((v2.c) >> 0) & 0xff), (((v2.c) >> 8) & 0xff), (((v2.c) >> 16) & 0xff),
		(((v3.c) >> 0) & 0xff), (((v3.c) >> 8) & 0xff), (((v3.c) >> 16) & 0xff),
		(((v4.c) >> 0) & 0xff), (((v4.c) >> 8) & 0xff), (((v4.c) >> 16) & 0xff)
	};
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
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

	// TODO: OpenGL implementation
	return;
}

void gpu::monochrome_line_opaque() {
	point v1, v2;
	v1.c = fifo[0] & 0xffffff;
	v2.c = fifo[0] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;

	// TODO: OpenGL implementation
	return;
}

void gpu::monochrome_rectangle_dot_opaque() {
	uint16_t x = fifo[1] & 0xffff;
	uint16_t y = fifo[1] >> 16;
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] 1x1 Draw Opaque Monochrome Rectangle (coords: %d;%d colour: 0x%x)\n", x, y, colour);
	if (x >= 640 || y >= 480)
		return;

	// TODO: OpenGL implementation
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