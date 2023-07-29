#pragma once

#include <helpers.hpp>
#include <backends/vertex.hpp>


class GPUBackend {
public:
	virtual void reset() { Helpers::panic("[FATAL] GPU Backend did not define reset function\n"); };
	virtual u8* getVRAM() { Helpers::panic("[FATAL] GPU Backend did not define getVRAM function\n"); };

	// Drawing
	virtual void drawTris(Vertex* v, u64 count) { Helpers::panic("[FATAL] GPU Backend did not define drawTris function\n"); }
};