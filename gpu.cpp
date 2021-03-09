#include "gpu.h"
#include <stdio.h>
gpu::gpu() {
	
}

void gpu::connectMem(memory* memory) {
	mem = *memory;
}

void gpu::execute(uint32_t command) {
	
	uint8_t instr = (command >> 24) & 0xff;
	if (instr == 0) {
		return;
	}
	
	switch (instr) {
	default:
		printf("\nUnknown GPU command: 0x%x\n", instr);
		exit(0);
	}
}
