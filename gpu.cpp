#include "gpu.h"
gpu::gpu() {
	debug = false;
	point v1, v2, v3, v4;
	//v1.x = 0; 
	//v1.y = 0;
	//v2.x = 640;
	//v2.y = 0;
	//v3.x = 0;
	//v3.y = 480;
	//v4.x = 640;
	//v4.y = 480;
	//
	//quad(v1, v2, v3, v4, 0xff00);
	
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


// trongles
void gpu::horizontal_line(point v1, point v2, uint32_t colour) {
	if (v1.x > v2.x) std::swap(v1, v2);
	uint16_t y = v1.y;
	for (int i = 0; i < 640; i++) {
		if (i >= v1.x && i <= v2.x) 
			pixels[y][i] = colour;
	}
}
void gpu::triangle(point v0, point v1, point v2, uint32_t colour) {
	const point* t = &v0;
	const point* m = &v1;
	const point* b = &v2;

	if (t->y > m->y) std::swap(t, m);
	if (m->y > b->y) std::swap(m, b);
	if (t->y > m->y) std::swap(t, m);

	float dy = (b->y - t->y);
	float iy = (m->y - t->y);

	if (m->y == t->y)	// top flat triangle
	{
		const point* l = m, * r = t;
		if (l->x > r->x) std::swap(l, r);
		top_flat_triangle(*l, *r, *b, colour);
	}
	else if (m->y == b->y) {	// bottom flat triangle
		const point* l = m, * r = b;
		if (l->x > r->x) std::swap(l, r);
		bottom_flat_triangle(*t, *l, *r, colour);
	}
	else {	// general case triangle
		point v4;
		v4.x = (int)(t->x + ((float)(m->y - t->y) / (float)(b->y - t->y)) * (b->x - t->x));
		v4.y = m->y;
		const point* l = m, * r = &v4;
		if (l->x > r->x) std::swap(l, r);
		bottom_flat_triangle(*t, *l, *r, colour);
		top_flat_triangle(*l, *r, *b, colour);
	}
	return;
}
void gpu::bottom_flat_triangle(point v0, point v1, point v2, uint32_t colour) {
	float invslope1 = (v1.x - v0.x) / (v1.y - v0.y);
	float invslope2 = (v2.x - v0.x) / (v2.y - v0.y);

	point _v1, _v2;
	for (int scanlineY = int(v0.y + 0.5f); scanlineY < int(v1.y + 0.5f); scanlineY++)
	{
		float dy = (scanlineY - v0.y) + 0.5f;
		float curx1 = v0.x + invslope1 * dy + 0.5f;
		float curx2 = v0.x + invslope2 * dy + 0.5f;


		int xl = std::max(1, (int)curx1);
		int xr = std::min(639, (int)curx2);
		_v1.x = xl;
		_v1.y = scanlineY;
		_v2.x = xr;
		_v2.y = scanlineY;
		horizontal_line(_v1, _v2, colour);
	}
}
void gpu::top_flat_triangle(point v0, point v1, point v2, uint32_t colour) {
	float invslope1 = (v2.x - v0.x) / (v2.y - v0.y);
	float invslope2 = (v2.x - v1.x) / (v2.y - v1.y);

	point _v1, _v2;
	for (int scanlineY = int(v2.y - 0.5f); scanlineY > int(v0.y - 0.5f); scanlineY--)
	{
		float dy = (scanlineY - v2.y) + 0.5f;
		float curx1 = v2.x + invslope1 * dy + 0.5f;
		float curx2 = v2.x + invslope2 * dy + 0.5f;


		int xl = std::max(0, (int)curx1);
		int xr = std::min(640, (int)curx2);
		_v1.x = xl;
		_v1.y = scanlineY;
		_v2.x = xr;
		_v2.y = scanlineY;
		horizontal_line(_v1, _v2, colour);
	}
}
void gpu::quad(point v1, point v2, point v3, point v4, uint32_t colour) {
	printf("\n(%d,%d)\n", v1.x, v1.y);
	printf("(%d,%d)\n", v2.x, v2.y);
	printf("(%d,%d)\n", v3.x, v3.y);
	printf("(%d,%d)\n\n", v4.x, v4.y);
	triangle(v1, v2, v3, colour);
	triangle(v2, v3, v4, colour);
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
		case(0x28): { // Monochrome four-point polygon, opaque
			fifo[0] = command;
			cmd_length++;
			cmd_left = 4;
			break;
		}
		case(0x30): {	// Shaded three-point polygon, opaque
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
			debug_printf("[GP0] Unknown GP0 command: 0x%x (0x%x)\n", instr, command);
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

				case(0x02): gpu::fill_rectangle(); break;
				case(0x20): gpu::monochrome_three_point_opaque_polygon(); break;
				case(0x28): gpu::monochrome_four_point_opaque_polygon(); break;
				case(0x30): gpu::shaded_three_point_opaque_polygon(); break;
				case(0x38): gpu::shaded_four_point_opaque_polygon(); break;
				case(0x68): gpu::monochrome_rectangle_dot_opaque(); break;
				case(0xA0): gpu::cpu_to_vram(); break;
				
				}
			}
			break;
		}
		case 1: {	// load mode
			debug_printf("[CPU to VRAM transfer] Data: 0x%x\n", command);
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
void gpu::monochrome_three_point_opaque_polygon() {
	debug_printf("[GP0] monochrome three-point polygon, opaque\n");
	point v1, v2, v3;
	uint32_t colour = fifo[0] & 0xffffff;
	v1.x = fifo[1] & 0xffff;
	v1.y = fifo[1] >> 16;
	v2.x = fifo[2] & 0xffff;
	v2.y = fifo[2] >> 16;
	v3.x = fifo[3] & 0xffff;
	v3.y = fifo[3] >> 16;
	triangle(v1, v2, v3, colour);
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
	triangle(v1, v2, v3, 0xff);
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
	debug_printf("[GP0] Shaded four-point polygon, opaque (rendered as mono, colour: 0x%x)\n", v1.c);
	quad(v1, v2, v3, v4, v1.c);
	return;
}
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
void gpu::fill_rectangle() {
	debug_printf("[GP0] Fill Rectangle");
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
