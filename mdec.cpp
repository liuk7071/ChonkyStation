#include "mdec.h"

mdec::mdec() {
	for (int i = 0; i < 64; i++) zagzig[zigzag[i]] = i;
	for (int i = 0; i < 0x50000; i++) output[i] = 0;
}

void mdec::command(uint32_t cmd) {
	if (parameters == 0) {
		status |= (1 << 31);
		switch (cmd >> 29) {
		case 7: // For some reason SimCity 2000 sends these... if I ignore them it seems to go on normally
		case 5:
		case 4:
		case 0: break;
		case 1: {	// Decode Macroblock(s)
			current_cmd = COMMAND::DECODE_MACROBLOCK;
			printf("[MDEC] Decode Macroblock(s)\n");
			printf("[MDEC] Macroblock depth: %d\n", (cmd >> 27) & 3);
			parameters = cmd & 0xffff;

			status &= (1 << 23);
			status |= ((cmd & (1 << 25)) >> 2); // Output bit15

			input.reserve(parameters * 2);
			break;
		}
		case 2: {	// Set Quant Table(s)
			qt_index = 0;
			current_cmd = COMMAND::SET_QUANT_TABLES;
			printf("[MDEC] Set Quant Table(s)\n");
			set_color_qt = cmd & 1;
			parameters = 64 * (set_color_qt ? 2 : 1); // Check if we also need to set the Color table
												      // Also this is the amount of bytes, so we need this line below
			parameters /= 4; // Divide by 4 because parameters are sent in 32bit words
			
			break;
		}
		case 3: {	// Set Scale Table
			st_index = 0;
			current_cmd = COMMAND::SET_SCALE_TABLE;
			printf("[MDEC] Set Scale Table\n");
			parameters = 64 / 2;  // Divide by 2 because 64 is the amount of signed halfwords and parameters are sent in 32bit words
			break;
		}
		default:
			printf("[MDEC] Unhandled command %d\n", cmd >> 29);
			exit(0);
		}
	}
	else {
		switch (current_cmd) {
		case COMMAND::DECODE_MACROBLOCK: {
			input.push_back(cmd & 0xffff);
			input.push_back(cmd >> 16);
			break;
		}
		case COMMAND::SET_QUANT_TABLES: {
			if (qt_index < 64) {
				luminance_qt[qt_index++] = (cmd >> 0) & 0xff;
				luminance_qt[qt_index++] = (cmd >> 8) & 0xff;
				luminance_qt[qt_index++] = (cmd >> 16) & 0xff;
				luminance_qt[qt_index++] = (cmd >> 24) & 0xff;
			}
			else if (set_color_qt && (qt_index < 128)) {
				color_qt[qt_index++ - 64] = (cmd >> 0) & 0xff;
				color_qt[qt_index++ - 64] = (cmd >> 8) & 0xff;
				color_qt[qt_index++ - 64] = (cmd >> 16) & 0xff;
				color_qt[qt_index++ - 64] = (cmd >> 24) & 0xff;
			}
			else {
				printf("[MDEC] Too many Set Quant Table(s) parameters\n");
				exit(0);
			}
			break;
		}
		case COMMAND::SET_SCALE_TABLE: {
			if (st_index < 64) {
				st[st_index++] = (int16_t)((cmd >> 0) & 0xffff);
				st[st_index++] = (int16_t)((cmd >> 16) & 0xffff);
			}
			else {
				printf("[MDEC] Too many Set Scale Table parameters\n");
				exit(0);
			}
			break;
		}
		}
		//printf("[MDEC] Parameter 0x%x\n", cmd);
		parameters--;
		if (parameters == 0 && (current_cmd == COMMAND::DECODE_MACROBLOCK)) decode_macroblock_15bpp();
	}
}

void mdec::decode_macroblock_15bpp() {
	output_index = 0;
	dma_out_index = 0;
	std::vector<uint16_t>::iterator src = input.begin();
	while (src != input.end()) {
		if (!rl_decode_block(cr, &src, color_qt)) break;
		if(!rl_decode_block(cb, &src, color_qt)) break;
		if(!rl_decode_block(y1, &src, luminance_qt)) break;
		if(!rl_decode_block(y2, &src, luminance_qt)) break;
		if(!rl_decode_block(y3, &src, luminance_qt)) break;
		if(!rl_decode_block(y4, &src, luminance_qt)) break;
		yuv_to_rgb(0, 0, y1);
		yuv_to_rgb(8, 0, y2);
		yuv_to_rgb(0, 8, y3);
		yuv_to_rgb(8, 8, y4);
		//printf("[MDEC] Decoded macroblock\n");
		output_index += 256 * 3;
		output_index = (output_index > 0x4fffff) ? (0x4fffff - 3) : output_index; // Quick way to avoiding overflowing... shouldn't ever happen but it does
	}
	output_index = 0;
	status &= ~(1 << 31);
	input.clear();
}

bool mdec::rl_decode_block(int16_t* blk, std::vector<uint16_t>::iterator* src, uint8_t* qt) {
	for (int i = 0; i < 64; i++) blk[i] = 0;
	uint16_t n = **src;
	while ((**src == 0xfe00) && (*src != input.end())) {
		(*src)++;
	}
	if (*src == input.end()) return false;
	int dest = 0;
	int32_t q_scale = (**src >> 10) & 0x3f;
	int32_t val = signed10bit(*(*src)++ & 0x3ff) * qt[0];
	while (dest < 64) {
		if (*src == input.end()) return false;
		if (q_scale == 0) val = signed10bit(n & 0x3ff) * 2;
		val = saturate(val, -0x400, 0x3ff);
		//val = val * scalezag[i];
		if (q_scale > 0) blk[zagzig[dest]] = val;
		else if (q_scale == 0) blk[dest] = val;

		n = **src;
		(*src)++;
		dest = dest + ((n >> 10) & 0x3f) + 1;
		val = (signed10bit(n & 0x3ff) * qt[dest] * q_scale + 4) / 8;
	}

	idct_core(blk);
	return true;
}

void mdec::idct_core(int16_t* blk) {
	int16_t* src = blk;
	int16_t* dstptr = &dst[0];

	for (int pass = 0; pass < 2; pass++) {
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				int sum = 0;
				for (int z = 0; z < 8; z++) {
					sum += src[y + z * 8] * (st[x + z * 8] / 8);
				}
				dstptr[x + y * 8] = (sum + 0xfff) / 0x2000;
			}
		}
		int16_t* tmp;
		tmp = src;
		src = dstptr;
		dstptr = tmp;
	}
}

void mdec::yuv_to_rgb(int xx, int yy, int16_t* yblk) {
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			int16_t R = cr[((x + xx) / 2) + ((y + yy) / 2) * 8];
			int16_t B = cb[((x + xx) / 2) + ((y + yy) / 2) * 8];
			int16_t G = (-0.3437 * B) + (-0.7143 * R);
			R = (1.402 * R);
			B = (1.772 * B);
			int16_t Y = yblk[x + y * 8];
			R = saturate(Y + R, -128, 127);
			G = saturate(Y + G, -128, 127);
			B = saturate(Y + B, -128, 127);
			R ^= 0x80; G ^= 0x80; B ^= 0x80;
			output[((x + xx) + (y + yy) * 16) * 3 + 0 + output_index] = R;
			output[((x + xx) + (y + yy) * 16) * 3 + 1 + output_index] = G;
			output[((x + xx) + (y + yy) * 16) * 3 + 2 + output_index] = B;
		}
	}
}