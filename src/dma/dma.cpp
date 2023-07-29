#include "dma.hpp"
#include <memory.hpp>


MAKE_LOG_FUNCTION(log, dmaLogger)

DMA::DMA() {
	channels[2].doDMA = &gpuDMA;
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

void DMA::gpuDMA(Memory* memory) {
	const auto& dma = memory->dma;
	auto& ch = dma->channels[2];
	log("Start GPU DMA\n");

	switch (ch.chcr.syncMode) {
	case (u32)SyncMode::Sync: {
		// We currently implement this one big whole transfer
		u32 addr = ch.madr & 0x1ffffc;
		// SyncMode 1 sets MADR to the start address of the currently transferred block
		// We fake this here
		const auto offset = (ch.bcr.ba - 1) * ch.bcr.bs;
		if (ch.chcr.step == (u32)Step::Forward)
			ch.madr += offset;
		else
			ch.madr -= offset;
			
		u32 words = ch.bcr.bs * ch.bcr.ba;
		// SyncMode 1 decrements BA to 0
		ch.bcr.ba = 0;

		u32 data = 0;
		switch (ch.chcr.dir) {
		case (u32)Direction::ToDevice: {
			while (words-- > 0) {
				data = memory->read<u32>(addr);
				memory->gpu->writeGp0(data);
				if (ch.chcr.step == (u32)Step::Forward)
					addr += 4;
				else
					addr -= 4;
				// TODO: Maybe mask with 0x1ffffc again after stepping?
			}
			break;
		}
		default:
			Helpers::panic("[DMA] Unimplemented GPU Sync DMA direction %d\n", ch.chcr.dir.Value());
		}
		break;
	}
	case (u32)SyncMode::LinkedList: {
		Helpers::debugAssert(ch.chcr.dir == (u32)Direction::ToDevice, "[FATAL] GPU LinkedList with direction ToRam");
		
		u32 header = 0;
		u32 words = 0;
		u32 data = 0;
		u32 addr = ch.madr & 0x1ffffc;
		while (true) {
			header = memory->read<u32>(addr);

			words = header >> 24;
			while (words-- > 0) {
				addr += 4;
				data = memory->read<u32>(addr);
				memory->gpu->writeGp0(data);
			}

			if (header & 0x800000)
				break;

			addr = header & 0x1ffffc;
		}

		// In SyncMode 2 (LinkedList), MADR is updated to hold the end marker
		ch.madr = header;
		break;
	}
	default:
		Helpers::panic("[DMA] Unimplemented GPU DMA sync mode %d\n", ch.chcr.syncMode.Value());
	}

	ch.chcr.enable = 0;
	ch.chcr.trigger = 0;
	// TODO: Update DICR
	log("GPU DMA done\n");
}

void DMA::otcDMA(Memory* memory) {
	const auto& dma = memory->dma;
	auto& ch = dma->channels[6];
	log("Start OTC DMA\n");

	// OTC DMA is always sync mode 0 and backwards memory address step and no chopping
	switch (ch.chcr.syncMode) {
	case (u32)SyncMode::Block: {
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