#pragma once

#include <helpers.hpp>
#include <logger.hpp>
#include <BitField.hpp>


// Circular dependency
class Memory;

class DMA {
public:
	DMA();

	struct DMAChannel {
		u32 madr = 0;

		union {
			u32 raw = 0;
			BitField<0,  16, u32> bc;
			BitField<0,  16, u32> bs;
			BitField<16, 16, u32> ba;
		} bcr;

		union {
			u32 raw = 0;
			BitField<0,  1, u32> dir;
			BitField<1,  1, u32> step;
			BitField<8,  1, u32> chopping;
			BitField<9,  2, u32> syncMode;
			BitField<16, 3, u32> choppingDmaSize;
			BitField<20, 3, u32> choppingCpuSize;
			BitField<24, 1, u32> enable;
			BitField<28, 1, u32> trigger;
		} chcr;

		bool shouldStartDMA();
		void (*doDMA)(Memory*) = nullptr;
	};

	DMAChannel channels[7];

	u32 dpcr = 0;
	u32 dicr = 0;

	enum class Direction {
		ToRam,
		ToDevice
	};
	enum class Step {
		Forward,
		Backward
	};
	enum class SyncMode {
		Block,
		Sync,
		LinkedList
	};

	void doDMA(int channel, Memory* memory);
	static void gpuDMA(Memory* memory);
	static void cdromDMA(Memory* memory);
	static void otcDMA(Memory* memory);
};