#include <cdrom.hpp>


CDROM::CDROM() {
	status.prmempt = 1;	// Parameter fifo empty
	status.prmwrdy = 1;	// Parameter fifo not full
}

void CDROM::executeCommand(u8 data) {
	Helpers::panic("[FATAL] Unimplemented CDROM command 0x%02x\n", data);

	Helpers::debugAssert(params.size() == 0, "[FATAL] CDROM command did not use all parameters");
	status.prmempt = 1;	// Parameter fifo empty
	status.prmwrdy = 1;	// Parameter fifo not full
}

void CDROM::pushParam(u8 data) {
	Helpers::debugAssert(params.size() <= 16, "[FATAL] Wrote more than 16 bytes to CDROM parameter fifo");
	params.push(data);
	if (params.size() == 16) {
		status.prmwrdy = 0;	// Parameter fifo full
	}
}

u8 CDROM::readStatus() {
	return status.raw;
}

void CDROM::writeStatus(u8 data) {
	status.index = data & 3;
}

void CDROM::writeIE(u8 data) {
	// TODO: stubbed
}

void CDROM::writeIF(u8 data) {
	// TODO: stubbed
}

u8 CDROM::getIndex() {
	return status.index;
}