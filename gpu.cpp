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
void gpu::putpixel(point v1, uint32_t colour) {
	Color c1(colour & 0xff, (colour & 0xff00) >> 8, (colour & 0xff0000) >> 16, 0);
	if (v1.x >= 640 || v1.y >= 480)
		return;

	pixels[v1.y * 640 + v1.x] = c1.ToUInt32();
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
		case(0x05): { // nop (?)
			debug_printf("[GP0] NOP\n");
			break;
		}
		case(0x08):	// nop
			debug_printf("[GP0] NOP (0x%x)\n", command);
			break;
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
		case(0xE1): {	// Dram Mode Setting
			debug_printf("[GP0] Mode Setting\n");
			page_base_x = command & 0xf;
			page_base_y = (command >> 4) & 1;
			semi_transparency = (command >> 5) & 3;
			// texture_depth
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
			auto width = resolution & 0xffff;
			auto height = resolution >> 16;

			int x = 0;
			int y = 0;
			if (cmd_left == 0) {	// load done
				gp0_mode = 0;
				break;
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
	point v1, v2, v3, v4;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[3] & 0xffff;
	v2.y = fifo[3] >> 16;
	v3.x = fifo[5] & 0xffff;
	v3.y = fifo[5] >> 16;
	v4.x = fifo[7] & 0xffff;
	v4.y = fifo[7] >> 16;
	quad(v1, v2, v3, v4, 0xff);
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
	pixels[y * 640 + x] = c1.ToUInt32();
	return;
}
void gpu::fill_rectangle() {
	debug_printf("[GP0] Fill Rectangle\n");
	return;
}
void gpu::cpu_to_vram() {
	debug_printf("[GP0] Copy Rectangle (CPU to VRAM)\n");
	uint32_t resolution = fifo[2];
	auto width = resolution & 0xffff;
	auto height = resolution >> 16;
	auto size = width * height;
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


void gpu::write32(uint32_t addrX, uint32_t addrY, uint32_t data) {
	//vram[addrY][addrX] = data & 0x000000ff;
	//vram[addrY][addrX] = data & 0xff000000 >> 24;
	//vram[addrY][addrX] = data & 0x00ff0000 >> 16;
	//vram[addrY][addrX] = data & 0x0000ff00 >> 8;
	return;
}
