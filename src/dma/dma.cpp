#include "dma.hpp"
#include <memory.hpp>


MAKE_LOG_FUNCTION(log, dmaLogger)

DMA::DMA() {
	channels[6].doDMA = &otcDMA;
}

bool DMA::DMAChannel::shouldStartDMA() {
	return chcr.enable && ((chcr.syncMode == 0) ? chcr.trigger.Value() : true);
}

void DMA::doDMA(int channel, Memory* memory) {
	// If the dma func of the channel is null, it means it hasn't been implemented yet
	if (!channels[channel].doDMA)
		Helpers::panic("[DMA] Unimplemented DMA channel %d\n", channel);
	
	channels[channel].doDMA(memory);
}

void DMA::otcDMA(Memory* memory) {
	const auto& dma = memory->dma;
	auto& ch = dma->channels[6];

	// OTC DMA is always sync mode 0 and backwards memory address step and no chopping
	switch (ch.chcr.syncMode) {
	case (u32)SyncMode::Block: {
		log("Start OTC DMA\n");
		u32 addr = ch.madr & 0x1ffffc;	// SyncMode 0 with chopping disabled doesn't update MADR
		u32 bc = ch.bcr.bc;				// SyncMode 0 with chopping disabled doesn't update BCR
		if (!bc) bc = 0x10000;			// If bc is 0, we need to transfer 0x10000 words
		while (bc-- > 1) {
			memory->write<u32>(addr, addr - 4);
			addr -= 4;	// OTC DMA is always decrementing
		}
		// Last word is 0xffffff
		memory->write<u32>(addr, 0xffffff);
		break;
	}
	default:
		Helpers::panic("[DMA] Unimplemented OTC DMA sync mode %d\n", ch.chcr.syncMode.Value());
	}

	ch.chcr.enable	= 0;
	ch.chcr.trigger = 0;
	// TODO: Update DICR
	log("OTC DMA done\n");
}