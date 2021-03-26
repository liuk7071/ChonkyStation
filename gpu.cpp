#include "gpu.h"
#include <stdio.h>
gpu::gpu() {
	debug = true;
}

void gpu::connectMem(memory* memory) {
	mem = *memory;
}

void gpu::execute_gp0(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;
	
	switch (instr) {
	case(0):
		if(debug) printf("[GPU] NOP (0x%x)\n", command);
		break;
	default:
		if(debug) printf("[GPU] Unknown GP0 command: 0x%x (0x%x)\n", instr, command);
		//exit(0);
	}
}

void gpu::execute_gp1(uint32_t command) {
	uint8_t instr = (command >> 24) & 0xff;

	switch (instr) {
	default:
		if(debug) printf("[GPU] Unknown GP1 command: 0x%x\n", instr);
		//exit(0);
	}
}
