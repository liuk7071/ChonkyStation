#include "spu.h"

std::ofstream file;

spu::spu() {
	for (int i = 0; i < 24; i++) {
		voices[i].ram = spu_ram;
	}
	file.open("sample", std::ios::binary);
}

void voice::on() {
	current_addr = adpcm_start * 8;
	adpcm_repeat = current_addr;
	enabled = true;
}

void voice::off() {
	// release
	enabled = false;
}

auto voice::step() {
	if (!enabled) return std::make_pair<std::optional<int16_t>, bool>(std::nullopt, false);
	
	bool endx = false;

	if (samples.empty()) {
		pitch_counter = 0;
		printf("Decoding samples at 0x%08x\n", current_addr);
		decode_samples(&ram[current_addr]);

		uint8_t flags = ram[current_addr + 1];
		const bool end = flags & 1;
		const bool repeat = (flags >> 1) & 1;
		const bool start = (flags >> 2) & 1;

	/*	if (!start && !end && !repeat) {
			current_addr += 16;
		}
		else if (!end && !repeat) {
			current_addr += 16;
			adpcm_repeat = current_addr;
		}
		else if (end) {
			current_addr = adpcm_repeat;
			if (!repeat) {
				// release here
				adsr_vol = 0;
			}
			endx = true;
		}*/
		
		if (start) adpcm_repeat = current_addr;
		current_addr += 16;	
		if (end) {
			endx = true;
			enabled = false;
			if (repeat) {
				current_addr = adpcm_repeat;
			}
			else {
				// release here
				adsr_vol = 0;
			}
		}
	}

	std::optional<int16_t> sample = samples.front();

	// Pitch counter
	auto old = pitch_counter >> 12;
	if (pitch > 0x3FFF) pitch = 0x4000;
	pitch_counter += pitch;
	if ((pitch_counter >> 12) > old) 
		samples.pop();
	return std::make_pair(sample, endx);
}

void voice::decode_samples(uint8_t* sample_start) {
	const auto shift = 12 - ((sample_start[0] & 0xf) > 12 ? 9 : (sample_start[0] & 0xf));
	const auto filter = (((sample_start[0]) >> 4) & 3);

	const auto f0 = pos_xa_adpcm_table[filter];
	const auto f1 = neg_xa_adpcm_table[filter];
	
	int8_t t = 0;
	int16_t s = 0;
	int sample_count = 0;
	for (int i = 2; i < 16; i++) {
		uint8_t byte = sample_start[i];

		t = byte & 0xf;
		t = (t << 4) >> 4;
		s = minmax((t << shift) + ((old * f0 + older * f1 + 32) / 64), -0x8000, 0x7fff);
		samples.push(s);
		older = old;
		old = s;
	
		t = ((byte >> 4) & 0xf);
		t = (t << 4) >> 4;
		s = minmax((t << shift) + ((old * f0 + older * f1 + 32) / 64), -0x8000, 0x7fff);
		samples.push(s);
		older = old;
		old = s;
	}
}

void spu::write(uint32_t addr, uint32_t data) {
	if	(addr >= 0x1f801c00 && (addr <= 0x1f801d7f)) {	// Voice regs
		int voice = (addr - 0x1f801c00) >> 4;
		switch (addr & 0xf) {
		case 0x0: voices[voice].volume_left = data; break;
		case 0x2: voices[voice].volume_right = data; break;
		case 0x4: voices[voice].pitch = data; break;
		case 0x6: voices[voice].adpcm_start = data; break;
		case 0x8: WRITE_LOWER(voices[voice].adsr, data); break;
		case 0xa: WRITE_UPPER(voices[voice].adsr, data); break;
		default:
			printf("Unhandled voice register write 0x%08x\n", addr);
			//exit(0);
		}
	} 
	else if (addr == 0x1f801d80) mainvolume_left = data;
	else if (addr == 0x1f801d82) mainvolume_right = data;
	else if (addr == 0x1f801d84) reverb_regs.vLOUT = data;
	else if (addr == 0x1f801d86) reverb_regs.rLOUT = data;
	else if (addr == 0x1f801d88) {
		WRITE_LOWER(key_on, data);
		for (int i = 0; i < 16; i++) {
			if ((key_on >> i) & 1) voices[i].on();
		}
	}
	else if (addr == 0x1f801d8a) {
		WRITE_UPPER(key_on, data);
		for (int i = 16; i < 24; i++) {
			if ((key_on >> i) & 1) voices[i].on();
		}
	}
	else if (addr == 0x1f801d8c) {
		WRITE_LOWER(key_off, data);
		for (int i = 0; i < 16; i++) {
			if ((key_off >> i) & 1) voices[i].off();
		}
	}
	else if (addr == 0x1f801d8e) {
		WRITE_UPPER(key_off, data);
		for (int i = 16; i < 24; i++) {
			if ((key_off >> i) & 1) voices[i].off();
		}
	}
	else if (addr == 0x1f801d90) WRITE_LOWER(pmod, data)
	else if (addr == 0x1f801d92) WRITE_UPPER(pmod, data)
	else if (addr == 0x1f801d94) {
		WRITE_LOWER(noise_mode, data);
		for (int i = 0; i < 16; i++) {
			if ((noise_mode >> i) & 1) printf("Voice %d noise mode\n", i);
			else printf("Voice %d ADPCM mode\n", i);
		}
	}
	else if (addr == 0x1f801d96) {
		WRITE_UPPER(noise_mode, data);
		for (int i = 16; i < 24; i++) {
			if ((noise_mode >> i) & 1) printf("Voice %d noise mode\n", i);
			else printf("Voice %d ADPCM mode\n", i);
		}
	}
	else if (addr == 0x1f801d98) {
		WRITE_LOWER(echo_on, data);
		for (int i = 0; i < 16; i++) {
			if ((echo_on >> i) & 1) printf("Voice %d reverb on\n", i);
			else printf("Voice %d reverb off\n", i);
		}
	}
	else if (addr == 0x1f801d9a) {
		WRITE_UPPER(echo_on, data);
		for (int i = 16; i < 24; i++) {
			if ((echo_on >> i) & 1) printf("Voice %d reverb on\n", i);
			else printf("Voice %d reverb off\n", i);
		}
	}
	else if (addr == 0x1f801da2) {
		reverb_regs.mBASE = data;
	}
	else if (addr == 0x1f801da6) {
		data_transfer_addr = data;
		current_transfer_addr = data_transfer_addr * 8;
	}
	else if (addr == 0x1f801da8) {
		//printf("SPU FIFO: 0x%04x\n", data);
		transfer_fifo.push_back(data);
	}
	else if (addr == 0x1f801daa) {
		spucnt = data;
		spustat &= ~0x3f;
		spustat |= spucnt & 0x3f;
		spustat &= ~0x80;
		spustat |= (spucnt & 0x20) << 2;

		if (spucnt >> 15) {
			printf("SPU enable\n");
			//exit(1);
		}
		
		// Handle manual writes
		if (((spucnt >> 4) & 3) == 1) {
			printf("Writing samples to SPU ram\n");
			for (int i = 0; i < transfer_fifo.size(); i++) {
				//spu_ram[current_transfer_addr] = transfer_fifo[i] & 0xff;
				//spu_ram[current_transfer_addr + 1] = (transfer_fifo[i] >> 8) & 0xff;
				*((uint16_t*)&spu_ram[current_transfer_addr]) = transfer_fifo[i];
				current_transfer_addr += 2;
			}
			transfer_fifo.clear();
		}
	}
	else if (addr == 0x1f801db0) printf("Unhandled SPU CD Audio Input Volume write lower\n");
	else if (addr == 0x1f801db2) printf("Unhandled SPU CD Audio Input Volume write upper\n");
	else if (addr == 0x1f801db4) printf("Unhandled SPU External Audio Input Volume write lower\n");
	else if (addr == 0x1f801db6) printf("Unhandled SPU External Audio Input Volume write upper\n");
	else if (addr == 0x1f801dac) {
		data_transfer_control = data;

		if (data != 4) {
			printf("Tried to write a value different from 4 to Sound RAM Data Transfer Control!\n");
			//exit(0);
		}
	}
	else if (addr == 0x1f801dc0) reverb_regs.dAPF1 = data;
	else if (addr == 0x1f801dc2) reverb_regs.dAPF2 = data;
	else if (addr == 0x1f801dc4) reverb_regs.vIIR = data;
	else if (addr == 0x1f801dc6) reverb_regs.vCOMB1 = data;
	else if (addr == 0x1f801dc8) reverb_regs.vCOMB2  = data;
	else if (addr == 0x1f801dca) reverb_regs.vCOMB3  = data;
	else if (addr == 0x1f801dcc) reverb_regs.vCOMB4  = data;
	else if (addr == 0x1f801dce) reverb_regs.vWALL   = data;
	else if (addr == 0x1f801dd0) reverb_regs.vAPF1   = data;
	else if (addr == 0x1f801dd2) reverb_regs.vAPF2   = data;
	else if (addr == 0x1f801dd4) reverb_regs.mLSAME  = data;
	else if (addr == 0x1f801dd6) reverb_regs.mRSAME  = data;
	else if (addr == 0x1f801dd8) reverb_regs.mLCOMB1 = data;
	else if (addr == 0x1f801dda) reverb_regs.mRCOMB1 = data;
	else if (addr == 0x1f801ddc) reverb_regs.mLCOMB2 = data;
	else if (addr == 0x1f801dde) reverb_regs.mRCOMB2 = data;
	else if (addr == 0x1f801de0) reverb_regs.dLSAME  = data;
	else if (addr == 0x1f801de2) reverb_regs.dRSAME  = data;
	else if (addr == 0x1f801de4) reverb_regs.mLDIFF  = data;
	else if (addr == 0x1f801de6) reverb_regs.mRDIFF  = data;
	else if (addr == 0x1f801de8) reverb_regs.mLCOMB3 = data;
	else if (addr == 0x1f801dea) reverb_regs.mRCOMB3 = data;
	else if (addr == 0x1f801dec) reverb_regs.mLCOMB4 = data;
	else if (addr == 0x1f801dee) reverb_regs.mRCOMB4 = data;
	else if (addr == 0x1f801df0) reverb_regs.dLDIFF  = data;
	else if (addr == 0x1f801df2) reverb_regs.dRDIFF  = data;
	else if (addr == 0x1f801df4) reverb_regs.mLAPF1  = data;
	else if (addr == 0x1f801df6) reverb_regs.mRAPF1  = data;
	else if (addr == 0x1f801df8) reverb_regs.mLAPF2  = data;
	else if (addr == 0x1f801dfa) reverb_regs.mRAPF2  = data;
	else if (addr == 0x1f801dfc) reverb_regs.vLIN    = data;
	else if (addr == 0x1f801dfe) reverb_regs.vRIN    = data;
	else {
		printf("Unhandled SPU write 0x%08x\n", addr);
		//exit(1);
	}
}

uint16_t spu::read(uint32_t addr) {
	if  (addr >= 0x1f801c00 && (addr <= 0x1f801d7f)) {	// Voice regs
		int voice = (addr - 0x1f801c00) >> 4;
		switch (addr & 0xf) {
		case 0x0: return voices[voice].volume_left;
		case 0x2: return voices[voice].volume_right;
		case 0x4: return voices[voice].pitch;
		case 0x6: return voices[voice].adpcm_start;
		case 0x8: return voices[voice].adsr;
		case 0xa: return voices[voice].adsr >> 16;
		case 0xc: return voices[voice].adsr_vol;
		default:
			printf("Unhandled voice register read 0x%08x\n", addr);
			//exit(0);
		}
	}
	else if	(addr == 0x1f801d88) return key_on;
	else if (addr == 0x1f801d8a) return key_on >> 16;
	else if (addr == 0x1f801d8c) return key_off;
	else if (addr == 0x1f801d8e) return key_off >> 16;
	else if (addr == 0x1f801da6) return data_transfer_addr;
	else if	(addr == 0x1f801daa) return spucnt;
	else if (addr == 0x1f801dac) return data_transfer_control;
	else if (addr == 0x1f801dae) return spustat;
	else {
		printf("Unhandled SPU read 0x%08x\n", addr);
		//exit(1);
	}
}

void spu::step(int cycles) {
	this->cycles += cycles;
	if (this->cycles > SAMPLE_RATE) {
		this->cycles = 0;

		// Mixer
		int16_t sample = 0;
		/*for (int i = 0; i < 24; i++) {
			if (voices[i].enabled) {
				auto [temp, endx] = voices[i].step();
				if (temp.has_value()) {
					sample += (temp.value() * 0.5);
					//mixed_samples++;
				}
			}
		}*/
		if (voices[2].enabled) sample = voices[2].step().first.value();
		//sample = minmax(sample, INT16_MIN, INT16_MAX);
		file.write((const char*)&sample, sizeof(int16_t));
	}
}