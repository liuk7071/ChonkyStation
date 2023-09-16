#include <cdrom.hpp>


CDROM::CDROM(Scheduler* scheduler) : scheduler(scheduler) {
	statusReg.prmempt = 1;	// Parameter fifo empty
	statusReg.prmwrdy = 1;	// Parameter fifo not full

	statusCode.motor = 1;	// Motor on
}

void CDROM::executeCommand(u8 data) {
	switch (data) {

	case CDROMCommands::GetStat: {
		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);
		break;
	}
	
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

	case CDROMCommands::GetID: {
		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);
		scheduler->push(&int5, scheduler->time + int3Delay + getIDDelay, this);
		break;
	}

	default:
		Helpers::panic("[FATAL] Unimplemented CDROM command 0x%02x\n", data);
	}

	Helpers::debugAssert(params.size() == 0, "[FATAL] CDROM command did not use all parameters");
	
	statusReg.rslrrdy = 1;	// Response fifo not empty
	statusReg.prmempt = 1;	// Parameter fifo empty
	statusReg.prmwrdy = 1;	// Parameter fifo not full
}

bool CDROM::shouldFireIRQ() {
	return intFlag & intEnable;
}

void CDROM::int3(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[FATAL] CDROM INT3 was fired before previous INT%d was acknowledged in interrupt flag\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 3;
}

void CDROM::int5(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[FATAL] CDROM INT5 was fired before previous INT%d was acknowledged in interrupt flag\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 5;

	// Second response
	if (cdrom->secondResponse.size()) {
		cdrom->response = cdrom->secondResponse;
		while (cdrom->secondResponse.size()) cdrom->secondResponse.pop();
	}
}

void CDROM::pushParam(u8 data) {
	Helpers::debugAssert(params.size() <= 16, "[FATAL] Wrote more than 16 bytes to CDROM parameter fifo");
	params.push(data);
	if (params.size() == 16) {
		statusReg.prmwrdy = 0;	// Parameter fifo full
	}
}

u8 CDROM::readStatus() {
	return statusReg.raw;
}

void CDROM::writeStatus(u8 data) {
	statusReg.index = data & 3;
}

void CDROM::writeIE(u8 data) {
	intEnable = data;
}

u8 CDROM::readIF() {
	return intFlag | (7 << 5);	// bits 5-7 are always 1
}

void CDROM::writeIF(u8 data) {
	intFlag &= ~(data & 0x1f);
	if (data & (1 << 6)) {	// "CLRPRM" clear parameter fifo
		while (params.size()) params.pop();
		statusReg.prmempt = 1;	// Parameter fifo empty
		statusReg.prmwrdy = 1;	// Parameter fifo not full
	}
	// Upon acknowledge, the response fifo is cleared
	while (response.size()) response.pop();
}

u8 CDROM::getIndex() {
	return statusReg.index;
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
		statusReg.rslrrdy = 0;	// Response fifo empty
	}
		
	return byte;
}