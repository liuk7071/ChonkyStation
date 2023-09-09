#include <cdrom.hpp>


CDROM::CDROM(Scheduler* scheduler) : scheduler(scheduler) {
	status.prmempt = 1;	// Parameter fifo empty
	status.prmwrdy = 1;	// Parameter fifo not full
}

void CDROM::executeCommand(u8 data) {
	switch (data) {
	
	case CDROMCommands::Test: {
		u8 subFunc = getParamByte();
		switch (subFunc) {
		case 0x20: {
			response.push(0x94);
			response.push(0x09);
			response.push(0x19);
			response.push(0xC0);
			break;
		}
		default:
			Helpers::panic("[FATAL] Unimplemented CDROM test subfunc 0x%02x\n", subFunc);
		}
		
		scheduler->push(&int3, scheduler->time + int3Delay, this);
		break;
	}

	default:
		Helpers::panic("[FATAL] Unimplemented CDROM command 0x%02x\n", data);
	}

	Helpers::debugAssert(params.size() == 0, "[FATAL] CDROM command did not use all parameters");
	
	status.rslrrdy = 1;	// Response fifo not empty
	status.prmempt = 1;	// Parameter fifo empty
	status.prmwrdy = 1;	// Parameter fifo not full
}

bool CDROM::shouldFireIRQ() {
	return intFlag & intEnable;
}

void CDROM::int3(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[FATAL] CDROM INT3 was fired before previous INT was acknowledged in interrupt flag (INT%d)\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 3;
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
	intEnable = data;
}

u8 CDROM::readIF() {
	return intFlag | (7 << 5);	// bits 5-7 are always 1
}

void CDROM::writeIF(u8 data) {
	intEnable &= ~(data & 0x1f);
	if (data & (1 << 6)) {	// "CLRPRM" clear parameter fifo
		while (params.size()) params.pop();
		status.prmempt = 1;	// Parameter fifo empty
		status.prmwrdy = 1;	// Parameter fifo not full
	}
	// Upon acknowledge, the response fifo is cleared
	while (response.size()) response.pop();
}

u8 CDROM::getIndex() {
	return status.index;
}

u8 CDROM::getParamByte() {
	u8 byte = params.front();
	params.pop();
	return byte;
}

u8 CDROM::getResponseByte() {
	if (!response.size()) {
		return 0;
	}

	u8 byte = response.front();
	response.pop();

	if (!response.size()) {
		status.rslrrdy = 0;	// Response fifo empty
	}
		
	return byte;
}