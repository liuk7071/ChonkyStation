#include "gpu.h"
#include "Bus.h"
#include <cmath>
Bus* bus;

gpu::gpu() {
	// Initialize pixel array
	pixels = new uint32_t[480 * 640];
	debug = false;
}

void connectBus(Bus *_bus) {
	bus = _bus;
}

#define LOG
#undef LOG

inline void gpu::debug_printf(const char* fmt, ...) {
#ifdef LOG
	if (debug) {
		std::va_list args;
		va_start(args, fmt);
		std::vprintf(fmt, args);
		va_end(args);
	}
#endif
}

uint32_t gpu::get_status() {
	uint32_t status = 0;

	status |= page_base_x << 0;
	status |= page_base_y << 3;

	return status;
}

void gpu::InitGL() {
	//WriteBuffer.reserve(1024 * 512);
	//WriteBuffer.clear();
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

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
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1024, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, VramTexture, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glPixelStorei(GL_PACK_ALIGNMENT, 2);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	glGenTextures(1, &SampleVramTexture);
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1024, 512);

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

	//int vramLocation = glGetUniformLocation(TextureShaderProgram, "vram");
	//int vram8Location = glGetUniformLocation(TextureShaderProgram, "vram8");
	//int vram4Location = glGetUniformLocation(TextureShaderProgram, "vram4");
	//colourDepthUniform = glGetUniformLocation(TextureShaderProgram, "colourDepth");
	//glUseProgram(TextureShaderProgram);
	//glUniform1i(vramLocation, 0);
	//glUniform1i(vram8Location, 1);
	//glUniform1i(vram4Location, 2);
}

void gpu::SyncVRAM() {
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
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
	glViewport(0, 0, 1024, 512);
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
		case 0x04:
		case 0x08:
		case 0x0c:
		case 0x10:
		case 0x14:
		case 0x18:
		case 0x1c:
		case 0x1d:
		case 0x00:	// nop
			debug_printf("[GP0] NOP (0x%x)\n", command);
			break;
		case 0x01: {	// Clear Cache
			SyncVRAM();
			debug_printf("[GP0] Clear Cache\n");
			break;
		}
		case 0x02: {	// Fill rectangle in VRAM
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x21:
		case 0x20: { // Monochrome three-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x23:
		case 0x22: { // Monochrome three-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x29:
		case 0x28: { // Monochrome four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 4;
			break;
		}
		case 0x2B:
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
		case 0x2D: { // Textured four-point polygon, opaque, raw-texture
			fifo[0] = command;
			cmd_length++;
			cmd_left = 8;
			break;
		}
		case 0x2E: { // Textured four-point polygon, semi-transparent, texture-blending
			cmd_length++;
			cmd_left = 8;
			break;
		}
		case 0x2F: { // Textured four-point polygon, semi-transparent, raw-texture
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
		case 0x33:
		case 0x32: { // Shaded three-point polygon, semi-transparent
			fifo[0] = command;
			cmd_length++;
			cmd_left = 5;
			break;
		}
		case 0x36:   // Shaded Textured three-point polygon, semi-transparent, tex-blend
		case 0x34: { // Shaded Textured three-point polygon, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 8;
			break;
		}
		case 0x38: { // Shaded four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 7;
			break;
		}
		case 0x3c: { // Shaded Textured four-point polygon, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 11;
			break;
		}
		case 0x3e: { // Shaded Textured four-point polygon, semi-transparent, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 11;
			break;
		}
		case 0x41:
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
			abort();
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
		case 0x62: { // Monochrome Rectangle (variable size) (semi-transparent)
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
		case 0x65: { // Textured Rectangle, variable size, opaque, raw-texture
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x66: { // Textured Rectangle, variable size, semi-transp, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0x67: { // Textured Rectangle, variable size, semi-transp, raw-texture
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
		case 0x70: { // Monochrome Rectangle (8x8) (opaque)
			fifo[0] = command;
			cmd_length++;
			cmd_left = 1;
			break;
		}
		case 0x74: { // Textured Rectangle, 8x8, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x75: { // Textured Rectangle, 8x8, opaque, raw-texture
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x7C: { // Textured Rectangle, 16x16, opaque, texture-blending
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x7D: { // Textured Rectangle, 16x16, opaque, raw-texture
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x7F: { // Textured Rectangle, 16x16, semi-transparent, raw-texture
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0x80: { // Copy Rectangle (VRAM to VRAM)
			fifo[0] = command;
			cmd_length++;
			cmd_left = 3;
			break;
		}
		case 0xA0: {	// Copy rectangle CPU to VRAM
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
			break;
		}
		case 0xC0: {	// Copy rectangle VRAM to CPU
			debug_printf("[GP0] Copy Rectangle (VRAM to CPU)\n");
			fifo[0] = command;
			cmd_length++;
			cmd_left = 2;
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
			int texWindow = command & 0xfffff;
			glUseProgram(TextureShaderProgram);
			int maskUniformLocation = glGetUniformLocation(TextureShaderProgram, "texWindow");

			int texWindowX = texWindow & 0x1f;
			int texWindowY = (texWindow >> 5) & 0x1f;
			int texWindowOffsX = (texWindow >> 10) & 0x1f;
			int texWindowOffsY = (texWindow >> 15) & 0x1f;
			glUniform4i(maskUniformLocation, ~(texWindowX * 8), ~(texWindowY * 8), ((texWindowOffsX& texWindowX) * 8), ((texWindowOffsY& texWindowY) * 8));
			debug_printf("[GP0] Set Texture Window\n");
			break;
		}
		case 0xE3: {	// Set Drawing Area top left
			drawing_topleft_x = command & 1023;
			drawing_topleft_y = (command >> 10) & 511;
			debug_printf("[GP0] Set Drawing Area top left\n");
			break;
		}
		case 0xE4: {	// Set Drawing Area bottom right
			drawing_bottomright_x = command & 1023;
			drawing_bottomright_y = (command >> 10) & 511;
			debug_printf("[GP0] Set Drawing Area bottom right\n");
			break;
		}
		case 0xE5: {	// Set Drawing Area Offset
			// unimplemented
			//printf("x offset: %d y offset: %d\n", (command & 0b1111111111), (command >> 11) & 0b1111111111);
			xoffset = (command & 0b1111111111);
			yoffset = ((command >> 11) & 0b1111111111);
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
			//abort();
		}
	}
	else {
		cmd_left--;
		switch (gp0_mode) {
		case 0: {	// command mode
			fifo[cmd_length++] = command;

			if (cmd_left == 0) {	// all the parameters are in, run command
				glScissor(drawing_topleft_x, drawing_topleft_y, (drawing_bottomright_x - drawing_topleft_x), (drawing_bottomright_y - drawing_topleft_y));
				glEnable(GL_SCISSOR_TEST);

				//SyncVRAM();

				switch ((fifo[0] >> 24) & 0xff) {

				case 0x02: gpu::fill_rectangle(); break;
				case 0x21:
				case 0x20: gpu::draw_untextured_tri(MONOCHROME, SOLID); break;
				case 0x23:
				case 0x22: gpu::draw_untextured_tri(MONOCHROME, SEMI_TRANSPARENT); break;
				case 0x29:
				case 0x28: gpu::draw_untextured_quad(MONOCHROME, SOLID); break;
				case 0x2B:
				case 0x2A: gpu::draw_untextured_quad(MONOCHROME, SEMI_TRANSPARENT); break;
				case 0x2C: gpu::texture_blending_four_point_opaque_polygon(); break;
				case 0x2D: gpu::texture_four_point_opaque_polygon(); break;
				case 0x2E: gpu::texture_blending_four_point_polygon_semi_transparent(); break;
				case 0x2F: gpu::texture_four_point_semi_transparent_polygon(); break;
				case 0x30: gpu::draw_untextured_tri(GOURAUD, SOLID); break;
				case 0x33:
				case 0x32: gpu::draw_untextured_tri(GOURAUD, SEMI_TRANSPARENT); break;
				case 0x36:
				case 0x34: gpu::shaded_texture_blending_three_point_opaque_polygon(); break;
				case 0x38: gpu::draw_untextured_quad(GOURAUD, SOLID); break;
				case 0x3A: gpu::draw_untextured_quad(GOURAUD, SEMI_TRANSPARENT); break;
				case 0x3C: gpu::shaded_texture_blending_textured_four_point_opaque_polygon(); break;
				case 0x3E: gpu::shaded_texture_blending_textured_four_point_semi_transparent_polygon(); break;
				case 0x41:
				case 0x40: gpu::monochrome_line_opaque(); break;
				case 0x60: gpu::monochrome_rectangle_variable_size_opaque(); break;
				case 0x62: gpu::monochrome_rectangle_variable_size_semi_transparent(); break;
				case 0x64: gpu::texture_blending_rectangle_variable_size_opaque(); break;
				case 0x65: gpu::texture_rectangle_variable_size_opaque(); break;
				case 0x66: gpu::texture_blending_rectangle_variable_size_semi_transparent(); break;
				case 0x67: gpu::textured_rectangle_variable_size_semi_transparent(); break;
				case 0x68: gpu::monochrome_rectangle_dot_opaque(); break;
				case 0x70: gpu::monochrome_rectangle_8x8_opaque(); break;
				case 0x74: gpu::texture_blending_rectangle_8x8_opaque(); break;
				case 0x75: gpu::texture_rectangle_8x8_opaque(); break;
				case 0x7C: gpu::texture_blending_rectangle_16x16_opaque(); break;
				case 0x7D: gpu::texture_rectangle_16x16_opaque(); break;
				case 0x7F: gpu::texture_rectangle_16x16_semi_transparent(); break;
				case 0x80: gpu::vram_to_vram(); break;
				case 0xA0: gpu::cpu_to_vram(); break;
				case 0xC0: gpu::vram_to_cpu(); break;
				//default: printf("\n%d", fifo[0] >> 24); abort();
				}
			}
			
			glDisable(GL_SCISSOR_TEST);
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

			if (cmd_left == 0) {
				gp0_mode = 0;
				glBindTexture(GL_TEXTURE_2D, VramTexture);
				glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, &WriteBuffer[0]);
				for (int i = 0; i < (1024 * 512) / 2; i++) WriteBuffer[i] = 0;
				//WriteBuffer.clear();
				WriteBufferCnt = 0;
				SyncVRAM();
				break;
			}
			//WriteBuffer.push_back(command);
			WriteBuffer[WriteBufferCnt++] = command;
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

void gpu::update_hres() {
	display_area.width = (((x2 - x1) / width_divisor) + 2) & ~3;
}
void gpu::update_vres() {
	display_area.height = y2 - y1;
	if (interlacing) display_area.height *= 2;
}

void gpu::execute_gp1(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;

	switch (instr) {
	case 0x0: {	// reset gpu
		hres1 = 1;
		int res_divisors[] = { 10, 8, 5, 4 };
		width_divisor = res_divisors[hres1]; // NTSC: 3413 video cycles per scanline

		x1 = 0x200;
		x2 = 0x200 + 256 * 10;
		y1 = 0x10;
		y2 = 0x10 + 240;
		update_hres();
		update_vres();
		debug_printf("[GP1] Reset Gpu\n");
		break;
	}
	case 0x1: // reset command buffer
		debug_printf("[GP1] Reset Command Buffer\n");
		break;
	case 0x4: { // set dma direction
		debug_printf("[GP1] Set DMA Direction to %d\n", command & 0b11);
		dma_direction = command & 0b11;
		break;
	}
	case 0x5: { // Start of Display area
		display_area.origin.x = command & 0x3ff;
		display_area.origin.y = (command >> 10) & 0x1ff;
		update_hres();
		update_vres();
		break;
	}
	case 0x6: { // Horizontal Display range
		x1 = command & 0xfff;
		x2 = (command >> 12) & 0xfff;
		update_hres();
		break;
	}
	case 0x7: { // Vertical Display range
		y1 = command & 0x3ff;
		y2 = (command >> 10) & 0x3ff;
		update_vres();
		break;
	}
	case 0x8: { // Display mode
		hres1 = command & 3;
		vres = (command >> 2) & 1;
		video_mode = (command >> 3) & 1;
		interlacing = (command >> 5) & 1;
		hres2 = (command >> 6) & 1;
		int res_divisors[] = { 10, 8, 5, 4 };
		width_divisor = res_divisors[hres1]; // NTSC: 3413 video cycles per scanline
		if (hres2) width_divisor = 7;
		update_hres();
		update_vres();
		break;
	}
	default:
		debug_printf("[GP1] Unknown GP1 command: 0x%x\n", instr);
		//exit(0);
	}
}

// commands

void gpu::draw_untextured_tri(int shading, int transparency) {
	if (shading == MONOCHROME) {
		point v1, v2, v3;
		uint32_t colour = fifo[0] & 0xffffff;
		v1.x = fifo[1] & 0xffff;
		v1.y = fifo[1] >> 16;
		v2.x = fifo[2] & 0xffff;
		v2.y = fifo[2] >> 16;
		v3.x = fifo[3] & 0xffff;
		v3.y = fifo[3] >> 16;

		v1.x += xoffset;
		v1.y += yoffset;
		v2.x += xoffset;
		v2.y += yoffset;
		v3.x += xoffset;
		v3.y += yoffset;

		uint32_t Vertices1[]{
			v1.x, v1.y, 0.0,
			v2.x, v2.y, 0.0,
			v3.x, v3.y, 0.0,

			(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
			(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
			(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
		};
		glViewport(0, 0, 1024, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
		glEnableVertexAttribArray(1);
		glUseProgram(ShaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	}
	else if (shading == GOURAUD) {
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

		v1.x += xoffset;
		v1.y += yoffset;
		v2.x += xoffset;
		v2.y += yoffset;
		v3.x += xoffset;
		v3.y += yoffset;

		uint32_t Vertices[]{
			v1.x, v1.y, 0.0,
			v2.x, v2.y, 0.0,
			v3.x, v3.y, 0.0,

			(((v1.c) >> 0) & 0xff), (((v1.c) >> 8) & 0xff), (((v1.c) >> 16) & 0xff),
			(((v2.c) >> 0) & 0xff), (((v2.c) >> 8) & 0xff), (((v2.c) >> 16) & 0xff),
			(((v3.c) >> 0) & 0xff), (((v3.c) >> 8) & 0xff), (((v3.c) >> 16) & 0xff)
		};
		glViewport(0, 0, 1024, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
		glEnableVertexAttribArray(1);
		glUseProgram(ShaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	}
}

void gpu::draw_untextured_quad(int shading, int transparency) {
	if (shading == MONOCHROME) {
		uint32_t colour = fifo[0] & 0xffffff;
		point v1, v2, v3, v4;
		v1.x = fifo[1] & 0xffff;
		v1.y = fifo[1] >> 16;
		v2.x = fifo[2] & 0xffff;
		v2.y = fifo[2] >> 16;
		v3.x = fifo[3] & 0xffff;
		v3.y = fifo[3] >> 16;
		v4.x = fifo[4] & 0xffff;
		v4.y = fifo[4] >> 16;

		v1.x += xoffset;
		v1.y += yoffset;
		v2.x += xoffset;
		v2.y += yoffset;
		v3.x += xoffset;
		v3.y += yoffset;
		v4.x += xoffset;
		v4.y += yoffset;

		uint32_t Vertices1[]{
			v1.x, v1.y, 0,
			v2.x, v2.y, 0,
			v3.x, v3.y, 0,

			(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
			(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
			(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
		};

		glViewport(0, 0, 1024, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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
		glViewport(0, 0, 1024, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
		glEnableVertexAttribArray(1);
		glUseProgram(ShaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	}
	else if (shading == GOURAUD) {
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

		v1.x += xoffset;
		v1.y += yoffset;
		v2.x += xoffset;
		v2.y += yoffset;
		v3.x += xoffset;
		v3.y += yoffset;
		v4.x += xoffset;
		v4.y += yoffset;

		uint32_t Vertices1[]{
			v1.x, v1.y, 0.0,
			v2.x, v2.y, 0.0,
			v3.x, v3.y, 0.0,

			(((v1.c) >> 0) & 0xff), (((v1.c) >> 8) & 0xff), (((v1.c) >> 16) & 0xff),
			(((v2.c) >> 0) & 0xff), (((v2.c) >> 8) & 0xff), (((v2.c) >> 16) & 0xff),
			(((v3.c) >> 0) & 0xff), (((v3.c) >> 8) & 0xff), (((v3.c) >> 16) & 0xff)
		};
		glViewport(0, 0, 1024, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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
		glViewport(0, 0, 1024, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
		glEnableVertexAttribArray(1);
		glUseProgram(ShaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	}
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
	//printf("\nclut: 0x%x\n", clut);
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
	//printf("texpage: 0x%x\n", texpage);
	uint32_t texpageX = ((texpage & 0b1111) * 64);
	uint32_t texpageY = (((texpage & 0b10000) >> 4) * 256);
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_four_point_opaque_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, opaque, raw textures (colour: 0x%x)\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_blending_four_point_polygon_semi_transparent() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, semi-transparent, texture-blending (colour: 0x%x)\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_four_point_semi_transparent_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, semi-transparent, raw-texture\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::shaded_texture_blending_three_point_opaque_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, opaque, raw textures (colour: 0x%x)\n", colour);
	point v1, v2, v3;
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
	texpage = fifo[5] >> 16;
	uint32_t texpageX = ((texpage & 0b1111) * 64);
	uint32_t texpageY = (((texpage & 0b10000) >> 4) * 256);
	v2.x = fifo[4] & 0xffff;
	v2.y = fifo[4] >> 16;
	v3.x = fifo[7] & 0xffff;
	v3.y = fifo[7] >> 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = (fifo[5] & 0xffff) & 0xff;
	t2.y = ((fifo[5] & 0xffff) >> 8) & 0xff;
	t3.x = (fifo[8] & 0xffff) & 0xff;
	t3.y = ((fifo[8] & 0xffff) >> 8) & 0xff;
	t4.x = (fifo[11] & 0xffff) & 0xff;
	t4.y = ((fifo[11] & 0xffff) >> 8) & 0xff;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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
}

void gpu::shaded_texture_blending_textured_four_point_opaque_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, opaque, raw textures (colour: 0x%x)\n", colour);
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
	texpage = fifo[5] >> 16;
	uint32_t texpageX = ((texpage & 0b1111) * 64);
	uint32_t texpageY = (((texpage & 0b10000) >> 4) * 256);
	v2.x = fifo[4] & 0xffff;
	v2.y = fifo[4] >> 16;
	v3.x = fifo[7] & 0xffff;
	v3.y = fifo[7] >> 16;
	v4.x = fifo[10] & 0xffff;
	v4.y = fifo[10] >> 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = (fifo[5] & 0xffff) & 0xff;
	t2.y = ((fifo[5] & 0xffff) >> 8) & 0xff;
	t3.x = (fifo[8] & 0xffff) & 0xff;
	t3.y = ((fifo[8] & 0xffff) >> 8) & 0xff;
	t4.x = (fifo[11] & 0xffff) & 0xff;
	t4.y = ((fifo[11] & 0xffff) >> 8) & 0xff;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::shaded_texture_blending_textured_four_point_semi_transparent_polygon() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured four-point polygon, opaque, raw textures (colour: 0x%x)\n", colour);
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
	texpage = fifo[5] >> 16;
	uint32_t texpageX = ((texpage & 0b1111) * 64);
	uint32_t texpageY = (((texpage & 0b10000) >> 4) * 256);
	v2.x = fifo[4] & 0xffff;
	v2.y = fifo[4] >> 16;
	v3.x = fifo[7] & 0xffff;
	v3.y = fifo[7] >> 16;
	v4.x = fifo[10] & 0xffff;
	v4.y = fifo[10] >> 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = (fifo[5] & 0xffff) & 0xff;
	t2.y = ((fifo[5] & 0xffff) >> 8) & 0xff;
	t3.x = (fifo[8] & 0xffff) & 0xff;
	t3.y = ((fifo[8] & 0xffff) >> 8) & 0xff;
	t4.x = (fifo[11] & 0xffff) & 0xff;
	t4.y = ((fifo[11] & 0xffff) >> 8) & 0xff;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

		glViewport(0, 0, 1024, 512);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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
	glViewport(0, 0, 1024, 512);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,
		v3.x, v3.y, 0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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
	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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

void gpu::monochrome_rectangle_variable_size_semi_transparent() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Monochrome Rectangle (variable size) (semi-transparent) (colour: 0x%x)\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,
		v3.x, v3.y, 0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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
	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_blending_rectangle_8x8_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, 8x8, opaque, texture-blending (colour: 0x%x)\n", colour);
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
	v2.x = v1.x + 8;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + 8;
	v4.x = v1.x + 8;
	v4.y = v1.y + 8;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = t1.x + 8;
	t2.y = t1.y;
	t3.x = t1.x;
	t3.y = t1.y + 8;
	t4.x = t1.x + 8;
	t4.y = t1.y + 8;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_rectangle_8x8_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, 8x8, opaque, raw-texture (colour: 0x%x)\n", colour);
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
	v2.x = v1.x + 8;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + 8;
	v4.x = v1.x + 8;
	v4.y = v1.y + 8;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = t1.x + 8;
	t2.y = t1.y;
	t3.x = t1.x;
	t3.y = t1.y + 8;
	t4.x = t1.x + 8;
	t4.y = t1.y + 8;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_blending_rectangle_16x16_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, 16x16, opaque, texture-blending (colour: 0x%x)\n", colour);
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
	v2.x = v1.x + 16;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + 16;
	v4.x = v1.x + 16;
	v4.y = v1.y + 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = t1.x + 16;
	t2.y = t1.y;
	t3.x = t1.x;
	t3.y = t1.y + 16;
	t4.x = t1.x + 16;
	t4.y = t1.y + 16;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_rectangle_16x16_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, 16x16, opaque, raw-texture (colour: 0x%x)\n", colour);
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
	v2.x = v1.x + 16;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + 16;
	v4.x = v1.x + 16;
	v4.y = v1.y + 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = t1.x + 16;
	t2.y = t1.y;
	t3.x = t1.x;
	t3.y = t1.y + 16;
	t4.x = t1.x + 16;
	t4.y = t1.y + 16;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_rectangle_16x16_semi_transparent() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, 16x16, semi-transparent, raw-texture (colour: 0x%x)\n", colour);
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
	v2.x = v1.x + 16;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + 16;
	v4.x = v1.x + 16;
	v4.y = v1.y + 16;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	point t1, t2, t3, t4;
	t1.x = (fifo[2] & 0xffff) & 0xff;
	t1.y = ((fifo[2] & 0xffff) >> 8) & 0xff;
	t2.x = t1.x + 16;
	t2.y = t1.y;
	t3.x = t1.x;
	t3.y = t1.y + 16;
	t4.x = t1.x + 16;
	t4.y = t1.y + 16;
	int colourDepth = (texpage >> 7) & 3;

	uint32_t Vertices1[] = {
		// positions          // colors																		   // texture coords
		 v1.x,  v1.y, 0,      (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t1.x, t1.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v2.x,  v2.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t2.x, t2.y,  texpageX, texpageY, clutX, clutY, colourDepth,
		 v3.x,  v3.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t3.x, t3.y,  texpageX, texpageY, clutX, clutY, colourDepth
		 //v4.x,  v4.y, 0.0f,   (((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),   t4.x, t4.y   // top left 
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_rectangle_variable_size_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, variable size, opaque, raw-texture (colour: 0x%x)\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::texture_blending_rectangle_variable_size_semi_transparent() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, variable size, semi-transp, texture-blending (colour: 0x%x)\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

void gpu::textured_rectangle_variable_size_semi_transparent() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Textured Rectangle, variable size, semi-transp, raw-texture (colour: 0x%x)\n", colour);
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

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, SampleVramTexture);
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
	uint32_t b = (((colour) >> 16) & 0xff);
	uint32_t g = (((colour) >> 8) & 0xff);
	uint32_t r = (((colour) >> 0) & 0xff);
	uint32_t rgba = (r << 24) | (g << 16) | (b << 8) | 0xff;
	vram_rgb[(y * 1024) + x] = rgba;

	// TODO: OpenGL implementation
	return;
}

void gpu::monochrome_rectangle_8x8_opaque() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Monochrome Rectangle (8x8) (opaque) (colour: 0x%x)\n", colour);
	point v1, res, v2, v3, v4;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = v1.x + 8;
	v2.y = v1.y;
	v3.x = v1.x;
	v3.y = v1.y + 8;
	v4.x = v1.x + 8;
	v4.y = v1.y + 8;

	v1.x += xoffset;
	v1.y += yoffset;
	v2.x += xoffset;
	v2.y += yoffset;
	v3.x += xoffset;
	v3.y += yoffset;
	v4.x += xoffset;
	v4.y += yoffset;

	uint32_t Vertices1[]{
		v1.x, v1.y, 0,
		v2.x, v2.y, 0,
		v3.x, v3.y, 0,

		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff),
		(((colour) >> 0) & 0xff), (((colour) >> 8) & 0xff), (((colour) >> 16) & 0xff)
	};

	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
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
	glViewport(0, 0, 1024, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
	glBindVertexArray(VAO);
	glVertexAttribIPointer(0, 3, GL_INT, 3 * sizeof(uint32_t), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(uint32_t), (void*)(9 * sizeof(uint32_t)));
	glEnableVertexAttribArray(1);
	glUseProgram(ShaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
}

void gpu::fill_rectangle() {
	debug_printf("[GP0] Fill Rectangle\n");
	glViewport(0, 0, 1024, 512);
	uint8_t r = (fifo[0] & 0xff);
	uint8_t g = ((fifo[0] >> 8) & 0xff);
	uint8_t b = ((fifo[0] >> 16) & 0xff);
	uint32_t x = fifo[1] & 0xffff;
	uint32_t y = fifo[1] >> 16;
	uint32_t width = fifo[2] & 0xffff;
	uint32_t height = fifo[2] >> 16;

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(r / 255.f, g / 255.f, b / 255.f, 1.f);
	glScissor(x, y, width, height);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	return;
}

void gpu::vram_to_vram() {
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] Copy Rectangle (VRAM to VRAM)\n", colour);
	uint32_t resolution = fifo[3];
	uint32_t coords = fifo[1];
	uint32_t dest_coords = fifo[2];
	auto width = resolution & 0xffff;
	auto height = resolution >> 16;
	if (width == 0) width = 1024;
	if (height == 0) height = 512;

	auto x = coords & 0x3ff;
	auto y = (coords >> 16) & 0x1ff;
	auto dest_x = dest_coords & 0x3ff;
	auto dest_y = (dest_coords >> 16) & 0x1ff;

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, ReadBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	glBindTexture(GL_TEXTURE_2D, VramTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, dest_x, dest_y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, ReadBuffer);
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
	cmd_left++;
	gp0_mode = 1;
}

void gpu::vram_to_cpu() {
	uint32_t resolution = fifo[2];
	uint32_t coords = fifo[1];
	auto width = resolution & 0xffff;
	auto height = resolution >> 16;
	if (width == 0) width = 1024;
	if (height == 0) height = 512;

	auto x = coords & 0x3ff;
	auto y = (coords >> 16) & 0x1ff;

	uint32_t size = width * height;
	size += 1;
	size &= ~1;

	size /= 2;

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, ReadBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

	ReadBufferCnt = 0;
}