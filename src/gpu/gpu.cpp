#include "gpu.hpp"


MAKE_LOG_FUNCTION(log, gpuLogger)

void GPU::scanlineEvent(void* classptr) {
	GPU* gpu = (GPU*)classptr;
	
	if (gpu->currentScanline < GPUConstants::scanlinesPerVdraw) {
		gpu->stat ^= 1 << 31;	// Interlacing
	}
	else if (gpu->currentScanline == GPUConstants::scanlinesPerVdraw) {
		gpu->vblank = true;
	}
	else if (gpu->currentScanline == GPUConstants::scanlinesPerFrame) {
		gpu->currentScanline = 0;
	}

	gpu->currentScanline++;

	gpu->scheduler->push(scanlineEvent, gpu->scheduler->time + GPUConstants::cyclesPerScanline, gpu);
}

u32 GPU::getStat() {
	// Stubbed
	stat |= 1 << 26;	// Ready to receive cmd word
	stat |= 1 << 27;	// Ready to send VRAM to CPU
	stat |= 1 << 28;	// Ready to receive DMA block
	stat |= 1 << 30;	// DMA direction CPU -> GP0
	return stat;
}

u32	GPU::gpuRead() {
	// Stubbed
	return 0;
}

void GPU::writeGp0(u32 data) {
	if (!hasCommand) {
		startCommand(data);
		fifo.push_back(data);
		return;
	}

	paramsLeft--;
	if (uploadingTexture) {
		backend->textureUploadData(data & 0xffff);
		backend->textureUploadData(data >> 16);
	}
	else {
		fifo.push_back(data);
	}

	if (paramsLeft == 0) {
		if (!uploadingTexture) {
			switch (fifo[0] >> 24) {
			case (u32)GP0Command::FillVRAM: {
				// TODO
				hasCommand = false;
				break;
			}
			case (u32)GP0Command::UploadTexture: {
				uploadingTexture = true;
				// Calculate size
				const u16 width = fifo[2] & 0xffff;
				const u16 height = fifo[2] >> 16;
				auto size = width * height;
				// Round
				size++;
				size &= ~1;
				// Each word contains two pixels, so we divide the amount of parameters left by 2
				paramsLeft = size / 2;
				uploadingTexture = true;

				const u16 x = fifo[1] & 0xffff;
				const u16 y = fifo[1] >> 16;
				backend->beginTextureUpload(x, y, width);

				log("  Width : %d\n", width);
				log("  Height: %d\n", height);
				log("  (x, y): (%d, %d)\n", x, y);
				break;
			}
			case (u32)GP0Command::ReadVRAM: {
				// TODO
				hasCommand = false;
				break;
			}
			default:
				hasCommand = false;

				DrawCommand drawCommand(fifo[0]);
				if (drawCommand.drawType == DrawCommand::DrawType::Polygon) {
					auto poly = drawCommand.getPolygon();
					auto nVerts = poly.quad ? 4 : 3;
					Vertex verts[4];
					std::memset(verts, 0, sizeof(Vertex) * 4);
					auto idx = 0;
					u32 col = fifo[idx++] & 0xffffff;
					verts[0].writeBGR888(col);
					verts[0].x = fifo[idx] & 0xffff;
					verts[0].y = (fifo[idx++] >> 16) & 0xffff;
					if (poly.textured) {
						const u16 uv = fifo[idx++];
						verts[0].u = uv & 0xff;
						verts[0].v = (uv >> 8) & 0xff;
					}

					for (int i = 1; i < nVerts; i++) {
						// Colour
						if (poly.shaded)
							verts[i].writeBGR888(fifo[idx++] & 0xffffff);
						else
							verts[i].writeBGR888(col);
						// Coords
						const u32 coords = fifo[idx++];
						verts[i].x = coords & 0xffff;
						verts[i].y = (coords >> 16) & 0xffff;
						// Texcoords
						if (poly.textured) {
							const u16 uv = fifo[idx++];
							verts[i].u = uv & 0xff;
							verts[i].v = (uv >> 8) & 0xff;
						}
					}

					// Draw
					if (!poly.textured) {
						backend->drawTriUntextured(verts[0], verts[1], verts[2]);
						if (poly.quad)
							backend->drawTriUntextured(verts[1], verts[2], verts[3]);
					}
					else {
						u16 clut = fifo[2] >> 16;
						u16 texpage = fifo[4] >> 16;
						backend->drawTriTextured(verts[0], verts[1], verts[2], clut, texpage);
						if (poly.quad)
							backend->drawTriTextured(verts[1], verts[2], verts[3], clut, texpage);
					}
				}
				else if (drawCommand.drawType == DrawCommand::DrawType::Rectangle) {
					auto rect = drawCommand.getRectangle();
					Vertex vert;
					auto idx = 0;
					u32 col = fifo[idx++] & 0xffffff;
					vert.writeBGR888(col);
					vert.x = fifo[idx] & 0xffff;
					vert.y = (fifo[idx++] >> 16) & 0xffff;
					if (rect.textured) {
						vert.u = fifo[idx] & 0xff;
						vert.v = (fifo[idx++] >> 8) & 0xff;
					}

					// TODO: textured
					Helpers::debugAssert(!rect.textured, "[FATAL] Unimplemented textured rectangle\n");

					u16 width = 0;
					u16 height = 0;

					if (rect.size == 0) {
						width = fifo[idx] & 0xffff;
						height = (fifo[idx++] >> 8) & 0xffff;
					}
					else {
						switch (rect.size) {
						case 1: width = 1; height = 1; break;
						case 2: width = 8; height = 8; break;
						case 3: width = 16; height = 16; break;
						}
					}

					backend->drawRectUntextured(vert, width, height);
				}
			}
		}
		else {
			backend->endTextureUpload();
			uploadingTexture = false;
			hasCommand = false;
		}
	}
}

void GPU::writeGp1(u32 data) {
	const auto cmd = (data >> 24) & 0xff;

	switch (cmd) {
	case (u32)GP1Command::ResetGPU: {
		stat = 0x14802000;
		break;
	}
	case (u32)GP1Command::ResetCommandBuffer: {
		// Stubbed
		break;
	}
	case (u32)GP1Command::AcknowledgeIRQ1: {
		stat &= ~(1 << 24);
		break;
	}
	case (u32)GP1Command::DisplayEnable: {
		// Bits 0 is copied to GPUSTAT.23
		stat &= ~(1 << 23);
		stat |= (data & 1) << 23;
		break;
	}
	case (u32)GP1Command::DMADirection: {
		// Bits 0-1 are copied to GPUSTAT.29-30
		stat &= ~(3 << 29);
		stat |= (data & 3) << 29;
		break;
	}
	case (u32)GP1Command::StartOfDisplayArea: {
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP1Command::HorizontalDisplayRange: {
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP1Command::VerticalDisplayRange: {
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP1Command::DisplayMode: {
		// Bits 0-5 are copied to GPUSTAT.17-22
		stat &= ~(0x3f << 17);
		stat |= (data & 0x3f) << 17;
		// Bit 6 is copied to GPUSTAT.16
		stat &= ~(1 << 16);
		stat |= (data & (1 << 6)) << 10;
		// Bit 7 is copied to GPUSTAT.17
		stat &= ~(1 << 14);
		stat |= (data & (1 << 7)) << 7;
		break;
	}
	case (u32)GP1Command::GetGPUInfo: {
		// TODO: STUBBED !!
		log("STUBBED GETGPUINFO!!!\n");
		break;
	}
	default:
		Helpers::panic("[GPU] Unimplemented gp1 command 0x%02x\n", cmd);
	}
}

void GPU::startCommand(u32 rawCommand) {
	// Clear FIFO
	fifo.clear();
	// We handle single-word commands (i.e. all the configuration ones) in this function
	const auto cmd = (rawCommand >> 24) & 0xff;
	switch (cmd) {
	case (u32)GP0Command::NOP: {
		log("NOP\n");
		// NOP
		break;
	}
	case (u32)GP0Command::ClearCache: {
		log("ClearCache\n");
		// Stubbed
		break;
	}
	case (u32)GP0Command::FillVRAM: {
		log("FillVRAM\n");
		paramsLeft = 2;
		hasCommand = true;
		break;
	}
	case (u32)GP0Command::UploadTexture: {
		log("UploadTexture\n");
		paramsLeft = 2;
		hasCommand = true;
		break;
	}
	case (u32)GP0Command::ReadVRAM: {
		log("ReadVRAM\n");
		paramsLeft = 2;
		hasCommand = true;
		break;
	}
	case (u32)GP0Command::DrawModeSetting: {
		log("DrawModeSetting\n");
		// Bits 0-10 are copied into GPUSTAT
		stat &= ~0x7ff;
		stat |= rawCommand & 0x7ff;
		// Bit 11 is copied into GPUSTAT.15
		stat &= ~(1 << 15);
		stat |= (rawCommand & (1 << 11)) << 4;
		break;
	}
	case (u32)GP0Command::TextureWindowSetting: {
		log("TextureWindowSetting\n");
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP0Command::SetDrawingAreaTopLeft: {
		log("SetDrawingAreaTopLeft\n");
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP0Command::SetDrawingAreaBottomRight: {
		log("SetDrawingAreaBottomRight\n");
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP0Command::SetDrawingOffset: {
		log("SetDrawingOffset\n");
		// TODO: Stubbed for now
		break;
	}
	case (u32)GP0Command::MaskBitSetting: {
		log("MaskBitSetting\n");
		// TODO: Stubbed for now
		break;
	}
	default: {
		DrawCommand drawCommand(rawCommand);
		hasCommand = true;
		paramsLeft = drawCommand.getCommandSize();
	}
	}
}

GPU::DrawCommand::DrawCommand(u32 raw) {
	this->raw = raw;
	switch ((raw >> 29) & 7) {
	
	// Polygon
	case 0b001: {
		Polygon temp = { .raw = raw };
		log("Polygon:\n");
		log(temp.quad ? "  Quad\n" : "  Tri\n");
		log(temp.shaded ? "  Shaded\n" : "  Monochrome\n");
		log(temp.textured ? "  Textured\n" : "  Untextured\n");
		drawType = DrawType::Polygon;
		break;
	}

	// Rectangle
	case 0b011: {
		// TODO: Logging
		drawType = DrawType::Rectangle;
		break;
	}

	default:
		Helpers::panic("[GPU] Unimplemented gp0 command 0x%02x (0x%08x)\n", raw >> 24, raw);
	}
}

// Returns the amount of words required for the command
u32 GPU::DrawCommand::getCommandSize() {
	u32 size = 1;
	if (drawType == DrawType::Polygon) {
		// Get number of words required per vertex
		Polygon poly = getPolygon();
		if (poly.shaded)   size++;
		if (poly.textured) size++;
		// Multiply by number of vertices
		size *= poly.quad ? 4 : 3;
		// First colour is included in the command word
		if (poly.shaded) size--;
	}
	else if (drawType == DrawType::Rectangle) {
		Rectangle rect = getRectangle();
		if (rect.textured)  size++;
		if (rect.size == 0) size++;
	}
	else {
		Helpers::panic("[GPU] Tried to get command size for unimplemented command type\n");
	}

	return size;
}

GPU::DrawCommand::Polygon GPU::DrawCommand::getPolygon() {
	if (drawType != DrawType::Polygon) Helpers::panic("[GPU] Tried to getPolygon but drawType was not polygon\n");
	return { .raw = this->raw };
}

GPU::DrawCommand::Rectangle GPU::DrawCommand::getRectangle() {
	if (drawType != DrawType::Rectangle) Helpers::panic("[GPU] Tried to getRectangle but drawType was not rectangle\n");
	return { .raw = this->raw };
}