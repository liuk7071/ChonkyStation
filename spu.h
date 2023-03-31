#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <optional>
#include <queue>

#define SAMPLE_RATE (33868800 / 44100)

#define WRITE_LOWER(reg, data)	{	\
	reg &= ~0xffff;					\
	reg |= (data);					\
}									\

#define WRITE_UPPER(reg, data) {	\
	reg &= ~(0xffff << 16);			\
	reg |= (data) << 16;			\
}									\

static int16_t minmax(int16_t val, int16_t min, int16_t max) {
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


struct voice {
	uint16_t volume_left = 0;
	uint16_t volume_right = 0;
	uint16_t sample_rate = 0;
	uint32_t adsr = 0;
	uint16_t adpcm_start = 0;
	uint16_t adpcm_repeat = 0;
	uint16_t current_addr = 0;
	uint16_t adsr_vol = 0;
	int cycles = 0;
	uint8_t* ram;
	void on();
	void off();
	auto step();
	bool enabled = false;
	void decode_samples(uint8_t* samples);
	int8_t pos_xa_adpcm_table[5] = { 0, 60, 115, 98, 122 };
	int8_t neg_xa_adpcm_table[5] = { 0, 0, -52, -55, -60 };
	int16_t old = 0;
	int16_t older = 0;
	std::queue<int16_t> samples;
};

class spu {
public:
	uint8_t* spu_ram = new uint8_t[0x80000];
	spu();
	void step(int cycles);
	int cycles = 0;
	
	voice* voices = new voice[24];

	void write(uint32_t addr, uint32_t data);
	uint16_t read(uint32_t addr);

	uint16_t mainvolume_left = 0;
	uint16_t mainvolume_right = 0;
	uint16_t spucnt = 0;
	uint16_t spustat = 0;

	uint32_t key_on = 0;
	uint32_t key_off = 0;
	uint32_t noise_mode = 0;
	uint32_t echo_on = 0;
	uint32_t pmod = 0;

	uint16_t data_transfer_control = 0;
	uint32_t data_transfer_addr = 0;
	uint32_t current_transfer_addr = 0;
	std::vector<uint16_t> transfer_fifo;

	struct {
		uint16_t vLOUT = 0;
		uint16_t rLOUT = 0;
		uint16_t mBASE = 0;
		uint16_t dAPF1 = 0;
		uint16_t dAPF2 = 0;
		uint16_t vIIR  = 0;
		uint16_t vCOMB1 = 0;
		uint16_t vCOMB2 = 0;
		uint16_t vCOMB3 = 0;
		uint16_t vCOMB4 = 0;
		uint16_t vWALL = 0;
		uint16_t vAPF1 = 0;
		uint16_t vAPF2 = 0;
		uint16_t mLSAME = 0;
		uint16_t mRSAME = 0;
		uint16_t mLCOMB1 = 0;
		uint16_t mRCOMB1 = 0;
		uint16_t mLCOMB2 = 0;
		uint16_t mRCOMB2 = 0;
		uint16_t dLSAME = 0;
		uint16_t dRSAME = 0;
		uint16_t mLDIFF = 0;
		uint16_t mRDIFF = 0;
		uint16_t mLCOMB3 = 0;
		uint16_t mRCOMB3 = 0;
		uint16_t mLCOMB4 = 0;
		uint16_t mRCOMB4 = 0;
		uint16_t dLDIFF = 0;
		uint16_t dRDIFF = 0;
		uint16_t mLAPF1 = 0;
		uint16_t mRAPF1 = 0;
		uint16_t mLAPF2 = 0;
		uint16_t mRAPF2 = 0;
		uint16_t vLIN = 0;
		uint16_t vRIN = 0;
	} reverb_regs;
};