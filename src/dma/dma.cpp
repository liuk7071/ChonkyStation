#include "dma.hpp"
#include <memory.hpp>


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
	auto& dma = memory->dma;
	constexpr auto ch = 6;

	// OTC DMA is always sync mode 0 and backwards memory address step
	switch (dma->channels[ch].chcr.syncMode) {
	default:
		Helpers::panic("[DMA] Unimplemented OTC DMA sync mode %d\n", dma->channels[ch].chcr.syncMode.Value());
	}
}