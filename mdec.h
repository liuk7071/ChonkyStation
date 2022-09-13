#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <vector>

class mdec {
public:
	mdec();
	void command(uint32_t cmd);
	enum class COMMAND {
		DECODE_MACROBLOCK,
		SET_QUANT_TABLES,
		SET_SCALE_TABLE
	};
	COMMAND current_cmd;
	bool set_color_qt = false;
	int parameters = 0;
	int qt_index = 0;
	int st_index = 0;
	int blk_index = 0;
	int current_loading_blk = 0;
	uint32_t status = (1 << 31);

	uint8_t luminance_qt[64] = { 0x02, 0x10, 0x10, 0x13, 0x10, 0x13, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x1a, 0x18, 0x1a, 0x1b, 0x1b, 0x1b, 0x1a, 0x1a, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1d, 0x1d, 0x1d, 0x22, 0x22, 0x22, 0x1d, 0x1d, 0x1d, 0x1b, 0x1b, 0x1d, 0x1d, 0x20, 0x20, 0x22, 0x22, 0x25, 0x26, 0x25, 0x23, 0x23, 0x22, 0x23, 0x26, 0x26, 0x28, 0x28, 0x28, 0x30, 0x30, 0x2e, 0x2e, 0x38, 0x38, 0x3a, 0x45, 0x45, 0x53, };
	uint8_t color_qt[64] = {0x02, 0x10, 0x10, 0x13, 0x10, 0x13, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x1a, 0x18, 0x1a, 0x1b, 0x1b, 0x1b, 0x1a, 0x1a, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1d, 0x1d, 0x1d, 0x22, 0x22, 0x22, 0x1d, 0x1d, 0x1d, 0x1b, 0x1b, 0x1d, 0x1d, 0x20, 0x20, 0x22, 0x22, 0x25, 0x26, 0x25, 0x23, 0x23, 0x22, 0x23, 0x26, 0x26, 0x28, 0x28, 0x28, 0x30, 0x30, 0x2e, 0x2e, 0x38, 0x38, 0x3a, 0x45, 0x45, 0x53 };
	int16_t st[64] = { 23170, 23170, 23170, 23170, 23170, 23170, 23170, 23170, 32138, 27245, 18204, 6392, -6393, -18205, -27246, -32139, 30273, 12539, -12540, -30274, -30274, -12540, 12539, 30273, 27245, -6393, -32139, -18205, 18204, 32138, 6392, -27246, 23170, -23171, -23171, 23170, 23170, -23171, -23171, 23170, 18204, -32139, 6392, 27245, -27246, -6393, 32138, -18205, 12539, -30274, 30273, -12540, -12540, 30273, -30274, 12539, 6392, -18205, 27245, -32139, 32138, -27246, 18204, -6393 };

	std::vector<uint16_t> input;
	uint8_t* output = new uint8_t[0x500000];
	int16_t cr[64] = { 0 };
	int16_t cb[64] = { 0 };
	int16_t y1[64] = { 0 };
	int16_t y2[64] = { 0 };
	int16_t y3[64] = { 0 };
	int16_t y4[64] = { 0 };
	int16_t dst[64];

	void decode_macroblock_15bpp();

	int zigzag[64] = {
		  0 ,1 ,5 ,6 ,14,15,27,28,
		  2 ,4 ,7 ,13,16,26,29,42,
		  3 ,8 ,12,17,25,30,41,43,
		  9 ,11,18,24,31,40,44,53,
		  10,19,23,32,39,45,52,54,
		  20,22,33,38,46,51,55,60,
		  21,34,37,47,50,56,59,61,
		  35,36,48,49,57,58,62,63
	};
	int zagzig[64];

	static int32_t signed10bit(int32_t n) { return (n << 22) >> 22; }
	static int16_t saturate(int16_t val, int16_t min, int16_t max) {
		if (val > max) {
			return max;
		}
		else if (val < min) {
			return min;
		}
		else {
			return val;
		}
	}

	bool rl_decode_block(int16_t* blk, std::vector<uint16_t>::iterator* src, uint8_t* qt);
	void idct_core(int16_t* blk);
	void yuv_to_rgb(int xx, int yy, int16_t* yblk);
	int output_index = 0;
	int dma_out_index = 0;
};