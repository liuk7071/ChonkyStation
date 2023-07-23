#pragma once

#include <helpers.hpp>
#include <BitField.hpp>


class DMA {
public:
	struct DMAChannel {
		u32 madr;

		union {
			u32 raw;
			BitField<0, 16, u32> bs;
			BitField<0, 16, u32> ba;
		} bcr;

		union {
			u32 raw;
			BitField<0,  1, u32> dir;
			BitField<1,  1, u32> step;
			BitField<8,  1, u32> chopping;
			BitField<9,  2, u32> syncMode;
			BitField<16, 3, u32> choppingDmaSize;
			BitField<20, 3, u32> choppingCpuSize;
			BitField<24, 1, u32> busy;
			BitField<28, 1, u32> trigger;
		} chcr;
	};

	u32 dpcr = 0;
	u32 dicr = 0;
};