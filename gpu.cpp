#include "gpu.h"
#include <stdio.h>
gpu::gpu() {
	debug = false;
}

void gpu::connectMem(memory* memory) {
	mem = *memory;
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
			printf("\n[GPU] Unknown GP0 command: 0x%x (0x%x)\n", instr, command);
			exit(0);
		}
	}
	else {
		cmd_left--;
		switch (gp0_mode) {
		case 0: {	// command mode
			fifo[cmd_length] = command;
			cmd_length++;

			if (cmd_left == 0) {	// all the parameters are in, run command

				switch ((fifo[0] >> 24) & 0xff) {

				case(0x68): gpu::monochrome_rectangle_dot_opaque(); break;
				case(0xA0): gpu::cpu_to_vram(); break;
				
				}
			}
			break;
		}
		case 1: {	// load mode
			printf("[CPU to VRAM transfer] Data: 0x%x\n", command);
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
	default:
		printf("\n[GPU] Unknown GP1 command: 0x%x\n", instr);
		//exit(0);
	}
}

// commands
void gpu::monochrome_rectangle_dot_opaque() {
	uint16_t x = fifo[1] & 0xffff;
	uint16_t y = fifo[1] >> 16;
	uint32_t colour = fifo[0] & 0xffffff;
	debug_printf("[GP0] 1x1 Draw Opaque Monochrome Rectangle (coords: %d;%d colour: 0x%x)\n", x, y, colour);
	if (x >= 640 || y >= 480)
		return;
	pixels[y][x] = colour;
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
}
