#pragma once

#include <helpers.hpp>
#include <backends/base.hpp>


class GPUSoftware : public GPUBackend {
public:
	void reset() override;
	u8* getVRAM() override;

private:
	u8* vram = new u8[1024 * 512];
};