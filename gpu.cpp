#include "gpu.h"
gpu::gpu() {
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
	//
	//quad(v1, v2, v3, v4, 0xff00);
	//triangle(v1, v2, v3, 0xff);
	
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
struct gpu::EdgeEquation {
	float a;
	float b;
	float c;
	bool tie;

	EdgeEquation(const point& v0, const point& v1)
	{
		a = v0.y - v1.y;
		b = v1.x - v0.x;
		c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) / 2;
		tie = a != 0 ? a > 0 : b > 0;
	}

	/// Evaluate the edge equation for the given point.

	float evaluate(float x, float y)
	{
		return a * x + b * y + c;
	}

	/// Test if the given point is inside the edge.

	bool test(float x, float y)
	{
		return test(evaluate(x, y));
	}

	/// Test for a given evaluated value.

	bool test(float v)
	{
		return (v > 0 || v == 0 && tie);
	}
};
struct gpu::ParameterEquation {
	float a;
	float b;
	float c;

	ParameterEquation(
		float p0,
		float p1,
		float p2,
		const EdgeEquation& e0,
		const EdgeEquation& e1,
		const EdgeEquation& e2,
		float area)
	{
		float factor = 1.0f / (2.0f * area);

		a = factor * (p0 * e0.a + p1 * e1.a + p2 * e2.a);
		b = factor * (p0 * e0.b + p1 * e1.b + p2 * e2.b);
		c = factor * (p0 * e0.c + p1 * e1.c + p2 * e2.c);
	}

	/// Evaluate the parameter equation for the given point.

	float evaluate(float x, float y)
	{
		return a * x + b * y + c;
	}
};


void gpu::putpixel(point v1, uint32_t colour) {
	if (v1.x >= 640 || v1.y >= 480)
		return;
	pixels[v1.y][v1.x] = colour;
}
void gpu::horizontal_line(point v1, point v2, uint32_t colour) {
	if (v1.x > v2.x) std::swap(v1, v2);
	uint16_t y = v1.y;
	for (int i = 0; i < 640; i++) {
		if (i >= v1.x && i <= v2.x) {
			point _v1;
			_v1.x = i;
			_v1.y = y;
			putpixel(_v1, colour);
		}
	}
}
void gpu::triangle(point v0, point v1, point v2, uint32_t colour) {
	// Compute triangle bounding box

	int minX = std::min(std::min(v0.x, v1.x), v2.x);
	int maxX = std::max(std::max(v0.x, v1.x), v2.x);
	int minY = std::min(std::min(v0.y, v1.y), v2.y);
	int maxY = std::max(std::max(v0.y, v1.y), v2.y);

	// Clip to scissor rect

	minX = std::max(minX, 0);
	maxX = std::min(maxX, 640);
	minY = std::max(minY, 0);
	maxY = std::min(maxY, 480);

	// Compute edge equations

	EdgeEquation e0(v0, v1);
	EdgeEquation e1(v1, v2);
	EdgeEquation e2(v2, v0);

	float area = 0.5 * (e0.c + e1.c + e2.c);

	ParameterEquation r(v0.r, v1.r, v2.r, e0, e1, e2, area);
	ParameterEquation g(v0.g, v1.g, v2.g, e0, e1, e2, area);
	ParameterEquation b(v0.b, v1.b, v2.b, e0, e1, e2, area);

	// Check if triangle is backfacing

	if (area < 0)
		return;

	for (float x = minX + 0.5f, xm = maxX + 0.5f; x <= xm; x += 1.0f)
		for (float y = minY + 0.5f, ym = maxY + 0.5f; y <= ym; y += 1.0f)
		{
			if (e0.test(x, y) && e1.test(x, y) && e2.test(x, y))
			{
				int rint = r.evaluate(x, y) * 255;
				int gint = g.evaluate(x, y) * 255;
				int bint = b.evaluate(x, y) * 255;
				uint32_t _colour = (bint << 16) | (gint << 8) | rint;
				point p;
				p.x = x;
				p.y = y;
				putpixel(p, colour);
			}
		}
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


		int xl = std::max(0, (int)curx1);
		int xr = std::min(640, (int)curx2);
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
