#include "gpu.h"
#include "Bus.h"
#include <cmath>
Bus* bus;

gpu::gpu() {
	// Initialize pixel array
	pixels = new uint32_t[480 * 640];
	debug = !true;
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
out vec4 frag_colour;

void main() {
	gl_Position = vec4(float(pos.x) / 320 - 1, -(1 - float(pos.y) / 240), 0.0, 1.0);
	frag_colour = vec4(float(colour.r) / 255, float(colour.g) / 255, float(colour.b) / 255, 255f);
}
)";
static const GLchar* TextureVertexShaderSource =
R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 texpage;
layout (location = 4) in vec2 clut;

out vec3 ourColor;
out vec2 TexCoord;
out vec2 _texCoord;
uniform int colourDepth;

void main()
{
    gl_Position = vec4(float(aPos.x) / 320 - 1, -(1 - float(aPos.y) / 240), 0.0, 1.0);
    ourColor = aColor;
	if(colourDepth == 0) {
		TexCoord = vec2(float((aTexCoord.x / 4) + texpage.x) / 1024 - 1, -(1 - float(aTexCoord.y + texpage.y) / 512));
	} else if(colourDepth == 1) {
		TexCoord = vec2(float((aTexCoord.x / 2) + texpage.x) / 1024 - 1, -(1 - float(aTexCoord.y + texpage.y) / 512));
	} else if(colourDepth == 2) {
		TexCoord = vec2(float(aTexCoord.x + texpage.x) / 1024 - 1, -(1 - float(aTexCoord.y + texpage.y) / 512));
	}
	_texCoord = aTexCoord;
}
)";
static const GLchar* FragmentShaderSource =
R"(
#version 330 core
in vec4 frag_colour;

out vec4 final_colour;

void main() {
	final_colour = frag_colour;
} 
)";

static const GLchar* TextureFragmentShaderSource =
R"(
#version 430 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec2 _texCoord;
layout(std430, binding = 10) buffer clutData
{
    int Clut[128];
};
uniform sampler2D vram;
uniform sampler2D vram8;
uniform sampler2D vram4;
uniform int colourDepth;

void main()
{
	vec4 pixel = vec4(0, 0, 0, 0);
	if(colourDepth == 0) {
		vec4 clutIdx = texture(vram4, TexCoord);
		int _pixel;
		if(int(_texCoord.x) % 4 == 0) {
			_pixel = Clut[int(clutIdx.r * 255)];
		}
		if(int(_texCoord.x) % 4 == 1) {
			_pixel = Clut[int(clutIdx.g * 255)];
		}
		if(int(_texCoord.x) % 4 == 2) {
			_pixel = Clut[int(clutIdx.b * 255)];
		}
		if(int(_texCoord.x) % 4 == 3) {
			_pixel = Clut[int(clutIdx.a * 255)];
		}
		pixel.r = (float(((_pixel >> 24) & 0xff)) / 255.0f);
		pixel.g = (float(((_pixel >> 16) & 0xff)) / 255.0f);
		pixel.b = (float(((_pixel >> 8) & 0xff)) / 255.0f);
		pixel.a = 1.0f;
	} else if(colourDepth == 1) {
		vec4 clutIdx = texture(vram8, TexCoord);
		int _pixel;
		if(int(_texCoord.x) % 2 == 0) {
			_pixel = Clut[int(clutIdx.r * 255)];
		}
		if(int(_texCoord.x) % 2 == 1) {
			_pixel = Clut[int(clutIdx.g * 255)];
		}
		pixel.r = (float(((_pixel >> 24) & 0xff)) / 255.0f);
		pixel.g = (float(((_pixel >> 16) & 0xff)) / 255.0f);
		pixel.b = (float(((_pixel >> 8) & 0xff)) / 255.0f);
		pixel.a = 1.0f;
	} else if(colourDepth == 2) {
		pixel = texture(vram, TexCoord);
	}
    if(pixel.r == 0 && pixel.g == 0 && pixel.b == 0) discard;
	
	FragColor = pixel;
}
)";

void gpu::InitGL() {
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, vram_rgb);

	glGenTextures(1, &VramTexture8);
	glBindTexture(GL_TEXTURE_2D, VramTexture8);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, vram8);

	glGenTextures(1, &VramTexture4);
	glBindTexture(GL_TEXTURE_2D, VramTexture4);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, vram4);

	int vramLocation = glGetUniformLocation(TextureShaderProgram, "vram");
	int vram8Location = glGetUniformLocation(TextureShaderProgram, "vram8");
	int vram4Location = glGetUniformLocation(TextureShaderProgram, "vram4");
	colourDepthUniform = glGetUniformLocation(TextureShaderProgram, "colourDepth");
	glUseProgram(TextureShaderProgram);
	glUniform1i(vramLocation, 0);
	glUniform1i(vram8Location, 1);
	glUniform1i(vram4Location, 2);
}

void gpu::putpixel(point v1, uint32_t colour) {
	// TODO: OpenGL implementation
	uint32_t a = (colour >> 15) & 1;
	uint32_t b = (colour >> 10) & 0b11111;
	uint32_t g = (colour >> 5) & 0b11111;
	uint32_t r = colour & 0b11111;
	uint32_t rgba = ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
	vram_rgb[(v1.y + ypos) * 1024 + (v1.x + xpos)] = rgba;
}

void gpu::ClearScreen() {
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

}

void gpu::execute_gp0(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;
	if (cmd_left == 0) {
		cmd_length = 0;
		switch (instr) {
		case 0x00:	// nop
			debug_printf("[GP0] NOP (0x%x)\n", command);
			break;
		case 0x01: {	// Clear Cache
			debug_printf("[GP0] Clear Cache\n");
			break;
		}
		case 0x02: {	// Fill rectangle in VRAM
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x20: { // Monochrome three-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x22: { // Monochrome three-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x28: { // Monochrome four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 4;
			break;
		}
		case 0x2A: { // Monochrome four-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 4;
			break;
		}
		case 0x2C: { // Textured four-point polygon, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 8;
			break;
		}
		case 0x30: {	// Shaded three-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 5;
			break;
		}
		case 0x32: { // Shaded three-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 5;
			break;
		}
		case 0x38: { // Shaded four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 7;
			break;
		}
		case 0x40: { // Monochrome line, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x3A: { // Shaded four-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 7;
			break;
		}
		case 0x48: { // Monochrome Poly-line, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 1;
			gp0_mode = 2;
			break;
		}
		case 0x60: { // Monochrome Rectangle (variable size) (opaque)
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x64: { // Textured Rectangle, variable size, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x68: {	// 1x1 Opaque Monochrome Rectangle
			fifo[0] = command;
			cmd_length++;
			cmd_left = 1;
			break;
		}
		case 0xA0: {	// Copy rectangle CPU to VRAM
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0xC0: {	// Copy rectangle VRAM to CPU
			// unimplemented
			debug_printf("[GP0] Copy Rectangle (VRAM to CPU)\n");
			break;
		}
		case 0xE1: {	// Draw Mode Setting
			debug_printf("[GP0] Draw Mode Setting\n");
			page_base_x = command & 0xf;
			page_base_y = (command >> 4) & 1;
			semi_transparency = (command >> 5) & 3;
			texture_depth = (command >> 7) & 3;
			dithering = ((command >> 9) & 1) != 0;
			allow_display_drawing = ((command >> 10) & 1) != 0;
			texpage_raw = command & 0xffff;
			// rectangle x flip
			// rectangle y flip
			break;
		}
		case 0xE2: {	// Set Texture Window
			// unimplemented
			debug_printf("[GP0] Set Texture Window\n");
			break;
		}
		case 0xE3: {	// Set Drawing Area top left
			// unimplemented
			debug_printf("[GP0] Set Drawing Area top left\n");
			break;
		}
		case 0xE4: {	// Set Drawing Area bottom right
			// unimplemented
			debug_printf("[GP0] Set Drawing Area bottom right\n");
			break;
		}
		case 0xE5: {	// Set Drawing Area Offset
			// unimplemented
			debug_printf("[GP0] Set Drawing Area Offset\n");
			break;
		}
		case 0xE6: {	// Set Mask Bit Setting
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

				case 0x02: gpu::fill_rectangle(); break;
				case 0x20: gpu::monochrome_three_point_opaque_polygon(); break;
				case 0x22: gpu::monochrome_three_point_semi_transparent_polygon(); break;
				case 0x28: gpu::monochrome_four_point_opaque_polygon(); break;
				case 0x2A: gpu::monochrome_four_point_semi_transparent_polygon(); break;
				case 0x2C: gpu::texture_blending_four_point_opaque_polygon(); break;
				case 0x30: gpu::shaded_three_point_opaque_polygon(); break;
				case 0x32: gpu::shaded_three_point_semi_transparent_polygon(); break;
				case 0x38: gpu::shaded_four_point_opaque_polygon(); break;
				case 0x3A: gpu::shaded_four_point_semi_transparent_polygon(); break;
				case 0x40: gpu::monochrome_line_opaque(); break;
				case 0x60: gpu::monochrome_rectangle_variable_size_opaque(); break;
				case 0x64: gpu::texture_blending_rectangle_variable_size_opaque(); break;
				case 0x68: gpu::monochrome_rectangle_dot_opaque(); break;
				case 0xA0: gpu::cpu_to_vram(); break;
				default: printf("\n%d", fifo[0] >> 24); exit(1);
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
			int index = ((y + ypos) * 1024 + (x + xpos));
			vram_rgb[index] = rgba;

			vram[index] = command & 0xffff;

			uint32_t pixel1 = (command & 0xff);
			uint32_t pixel2 = ((command >> 8) & 0xff);
			vram8[index] = (pixel2 << 8) | pixel1;

			pixel1 = (command & 0xf);
			pixel2 = ((command >> 4) & 0xf);
			uint32_t pixel3 = ((command >> 8) & 0xf);
			uint32_t pixel4 = ((command >> 12) & 0xf);
			vram4[index] = (pixel4 << 24) | (pixel3 << 16) | (pixel2 << 8) | pixel1;

			xpos++;

			if (xpos == width) {
				xpos = 0;
				ypos++;

				if (ypos == height) {
					gp0_mode = 0;
					glBindTexture(GL_TEXTURE_2D, VramTexture);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, vram_rgb);
					glBindTexture(GL_TEXTURE_2D, VramTexture8);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, vram8);
					glBindTexture(GL_TEXTURE_2D, VramTexture4);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, vram4);
					break;
				}
			}

			c = command >> 16;
			a = (c >> 15) & 1;
			b = (c >> 10) & 0b11111;
			g = (c >> 5) & 0b11111;
			r = c & 0b11111;
			rgba = ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
			index = ((y + ypos) * 1024 + (x + xpos));
			vram_rgb[index] = rgba;

			vram[index] = (command >> 16);

			pixel1 = ((command >> 16) & 0xff);
			pixel2 = (((command >> 16) >> 8) & 0xff);
			vram8[index] = (pixel2 << 8) | pixel1;

			pixel1 = ((command >> 16) & 0xf);
			pixel2 = (((command >> 16) >> 4) & 0xf);
			pixel3 = (((command >> 16) >> 8) & 0xf);
			pixel4 = (((command >> 16) >> 12) & 0xf);
			vram4[index] = (pixel4 << 24) | (pixel2 << 16) | (pixel3 << 8) | pixel1;
			xpos++;

			if (xpos == width) {
				xpos = 0;
				ypos++;

				if (ypos == height) {
					gp0_mode = 0;
					glBindTexture(GL_TEXTURE_2D, VramTexture);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, vram_rgb);
					glBindTexture(GL_TEXTURE_2D, VramTexture8);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, vram8);
					glBindTexture(GL_TEXTURE_2D, VramTexture4);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, vram4);
					break;
				}
			}
			break;
		}
		case 2: {	// polyline mode
			if (command == 0x50005000 || command == 0x55555555) {
				monochrome_polyline_opaque();
				cmd_length = 0;
				gp0_mode = 0;
				cmd_left = 0;
				return;
			}
			cmd_left += 2;
			fifo[cmd_length++] = command;
			break;
		}
		}
	}
}

void gpu::execute_gp1(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;

	switch (instr) {
	case 0x0:	// reset gpu
		debug_printf("[GP1] Reset Gpu\n");
		break;
	case 0x1: // reset command buffer
		debug_printf("[GP1] Reset Command Buffer\n");
		break;
	case 0x4: { // set dma direction
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
	uint32_t clutX = (clut & 0x3f);
	clutX *= 16;
	uint32_t clutY = (clut >> 6);
	int Clut[256];
	for (int i = 0; i < 256; i++) {
		Clut[i] = vram_rgb[clutY * 1024 + clutX + i];
	}
	GLuint ssbo;
	GLuint binding = 10;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * sizeof(int), Clut, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
	texpage = fifo[4] >> 16;
	uint32_t texpageX = ((texpage & 0b1111) * 64);
	uint32_t texpageY = (((texpage & 0b10000) >> 4) * 256);
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = (fifo[4] & 0xffff) & 0xff;
	t2.y = ((fifo[4] & 0xffff) >> 8) & 0xff;
	t3.x = (fifo[6] & 0xffff) & 0xff;
	t3.y = ((fifo[6] & 0xffff) >> 8) & 0xff;
	t4.x = (fifo[8] & 0xffff) & 0xff;
	t4.y = ((fifo[8] & 0xffff) >> 8) & 0xff;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, VramTexture);
	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, VramTexture8);
	glActiveTexture(GL_TEXTURE0 + 2); // Texture unit 2
	glBindTexture(GL_TEXTURE_2D, VramTexture4);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	// Colour attribute
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(3 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(6 * sizeof(uint32_t)));
	glEnableVertexAttribArray(2);
	// texpage attribute
	glVertexAttribPointer(3, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(8 * sizeof(uint32_t)));
	glEnableVertexAttribArray(3);
	// clut attribute
	glVertexAttribPointer(4, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(10 * sizeof(uint32_t)));
	glEnableVertexAttribArray(4);
	// colour depth attribute
	glVertexAttribPointer(5, 1, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(12 * sizeof(uint32_t)));
	glEnableVertexAttribArray(5);
	glUseProgram(TextureShaderProgram);
	glUniform1i(colourDepthUniform, colourDepth);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	uint32_t Vertices2[] = {
		// positions          // colors																		   // texture coords // texpage
		 v2.x,  v2.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,		 texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,		 texpageX, texpageY, clutX, clutY, colourDepth,
		 v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y,		 texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, VramTexture);
	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, VramTexture8);
	glActiveTexture(GL_TEXTURE0 + 2); // Texture unit 2
	glBindTexture(GL_TEXTURE_2D, VramTexture4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	// Colour attribute
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(3 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(6 * sizeof(uint32_t)));
	glEnableVertexAttribArray(2);
	// texpage attribute
	glVertexAttribPointer(3, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(8 * sizeof(uint32_t)));
	glEnableVertexAttribArray(3);
	// clut attribute
	glVertexAttribPointer(4, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(10 * sizeof(uint32_t)));
	glEnableVertexAttribArray(4);
	// colour depth attribute
	glVertexAttribPointer(5, 1, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(12 * sizeof(uint32_t)));
	glEnableVertexAttribArray(5);
	glUseProgram(TextureShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
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

void gpu::monochrome_polyline_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Monochrome four-point polygon, opaque (colour: 0x%x)\n", colour);
	point v1, v2, v3, v4, v5, v6;
	/*for (int i = 1; i < cmd_length; i++) {
		v1.x = fifo[i] & 0xffff;
		v1.y = fifo[i] >> 16;
		v2.x = fifo[i+1] & 0xffff;
		v2.y = fifo[i+1] >> 16;

		uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
		};

		glViewport(0, 0, 640, 480);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribPointer(0, 2, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(6 * sizeof(uint32_t)));
		glEnableVertexAttribArray(1);
		glUseProgram(ShaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE, 0, 3);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	}*/
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;
	v3.x = fifo[3] & 0xffff;
	v3.y = fifo[3] >> 16;
	v4.x = fifo[4] & 0xffff;
	v4.y = fifo[4] >> 16;
	v5.x = fifo[5] & 0xffff;
	v5.y = fifo[5] >> 16;

	//printf("%d\n", cmd_length);
	uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,
		v3.x, v3.y, 0,
		v4.x, v4.y, 0,
		v5.x, v5.y, 0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
	};

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(15 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	/*uint32_t Vertices2[]{
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
	glDrawArrays(GL_LINE_STRIP, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);*/
	return;
}

void gpu::monochrome_rectangle_variable_size_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Monochrome Rectangle (variable size) (opaque) (colour: 0x%x)\n", colour);
	point v1, res, v2, v3, v4;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	res.x = fifo[2] & 0xffff;
	res.y = fifo[2] >> 16;
	v2.x = v1.x + res.x;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + res.y;
	v4.x = v1.x + res.x;
	v4.y = v1.y + res.y;

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

void gpu::texture_blending_rectangle_variable_size_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, variable size, opaque, texture-blending (colour: 0x%x)\n", colour);
	point v1, res, v2, v3, v4;
	uint16_t texpage = texpage_raw;
	uint16_t clut = 0;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	clut = fifo[2] >> 16;
	uint32_t clutX = (clut & 0x3f);
	clutX *= 16;
	uint32_t clutY = (clut >> 6);
	int Clut[256];
	for (int i = 0; i < 256; i++) {
		Clut[i] = vram_rgb[clutY * 1024 + clutX + i];
	}
	GLuint ssbo;
	GLuint binding = 10;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * sizeof(int), Clut, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
	//texpage = fifo[4] >> 16;
	uint32_t texpageX = ((texpage & 0b1111) * 64);
	uint32_t texpageY = (((texpage & 0b10000) >> 4) * 256);
	res.x = fifo[3] & 0xffff;
	res.y = fifo[3] >> 16;
	v2.x = v1.x + res.x;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + res.y;
	v4.x = v1.x + res.x;
	v4.y = v1.y + res.y;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = t1.x + res.x;
	t2.y = t1.y;
	t3.x = t1.x;
	t3.y = t1.y + res.y;
	t4.x = t1.x + res.x;
	t4.y = t1.y + res.y;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, VramTexture);
	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, VramTexture8);
	glActiveTexture(GL_TEXTURE0 + 2); // Texture unit 2
	glBindTexture(GL_TEXTURE_2D, VramTexture4);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	// Colour attribute
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(3 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(6 * sizeof(uint32_t)));
	glEnableVertexAttribArray(2);
	// texpage attribute
	glVertexAttribPointer(3, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(8 * sizeof(uint32_t)));
	glEnableVertexAttribArray(3);
	// clut attribute
	glVertexAttribPointer(4, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(10 * sizeof(uint32_t)));
	glEnableVertexAttribArray(4);
	// colour depth attribute
	glVertexAttribPointer(5, 1, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(12 * sizeof(uint32_t)));
	glEnableVertexAttribArray(5);
	glUseProgram(TextureShaderProgram);
	glUniform1i(colourDepthUniform, colourDepth);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	uint32_t Vertices2[] = {
		// positions          // colors																		   // texture coords // texpage
		 v2.x,  v2.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,		 texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,		 texpageX, texpageY, clutX, clutY, colourDepth,
		 v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y,		 texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, VramTexture);
	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, VramTexture8);
	glActiveTexture(GL_TEXTURE0 + 2); // Texture unit 2
	glBindTexture(GL_TEXTURE_2D, VramTexture4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	// Colour attribute
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(3 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(6 * sizeof(uint32_t)));
	glEnableVertexAttribArray(2);
	// texpage attribute
	glVertexAttribPointer(3, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(8 * sizeof(uint32_t)));
	glEnableVertexAttribArray(3);
	// clut attribute
	glVertexAttribPointer(4, 2, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(10 * sizeof(uint32_t)));
	glEnableVertexAttribArray(4);
	// colour depth attribute
	glVertexAttribPointer(5, 1, GL_UNSIGNED_INT, GL_FALSE, 13 * sizeof(uint32_t), (void*)(12 * sizeof(uint32_t)));
	glEnableVertexAttribArray(5);
	glUseProgram(TextureShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
}

void gpu::monochrome_rectangle_dot_opaque() {
	uint16_t x = fifo[1] & 0xffff;
	uint16_t y = fifo[1] >> 16;
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] 1x1 Draw Opaque Monochrome Rectangle (coords: %d;%d colour: 0x%x)\n", x, y, colour);
	uint32_t a = (colour >> 15) & 1;
	uint32_t b = (colour >> 10) & 0b11111;
	uint32_t g = (colour >> 5) & 0b11111;
	uint32_t r = colour & 0b11111;
	uint32_t rgba = ((r << 3) << 24) | ((g << 3) << 16) | ((b << 3) << 8) | 0xff;
	vram_rgb[(y * 1024) + x] = rgba;

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