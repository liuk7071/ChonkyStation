#include <cdrom.hpp>


MAKE_LOG_FUNCTION(log, cdromLogger)

CDROM::CDROM(const fs::path& cdPath, Scheduler* scheduler) : scheduler(scheduler) {
	std::string pathStr = cdPath.string();
	cd = std::fopen(pathStr.c_str(), "rb");

	statusReg.prmempt = 1;	// Parameter fifo empty
	statusReg.prmwrdy = 1;	// Parameter fifo not full

	statusCode.motor = 1;	// Motor on
}

void CDROM::executeCommand(u8 data) {
	switch (data) {

	case CDROMCommands::GetStat: {
		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);

		log("GetStat (stat: 0x%02x)\n", statusCode.raw);
		break;
	}

	case CDROMCommands::SetLoc: {
		u8 mm = bcdToDec(getParamByte());
		u8 ss = bcdToDec(getParamByte());
		u8 ff = bcdToDec(getParamByte());

		seekLoc = ff + (ss * 75) + (mm * 60 * 75);

		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);

		log("SetLoc (loc: %d)\n", seekLoc);
		break;
	}

	case CDROMCommands::ReadN: {
		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);

		log("ReadN\n");

		beginReading();
		scheduler->push(&cdRead, scheduler->time + (!mode.doubleSpeed ? readTime : readTimeDoubleSpeed), this, "cdRead");

		break;
	}

	case CDROMCommands::Pause: {
		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);
		
		scheduler->push(&int2, scheduler->time + int3Delay + int2Delay, this);
		stopReading();
		secondResponse.push(statusCode.raw);

		log("Pause\n");
		break;
	}

	case CDROMCommands::Init: {
		response.push(statusCode.raw);
		secondResponse.push(statusCode.raw);

		scheduler->push(&int3, scheduler->time + int3Delay, this);
		scheduler->push(&int2, scheduler->time + int3Delay + int2Delay, this);

		log("Init\n");
		break;
	}

	case CDROMCommands::Setmode: {
		mode.raw = getParamByte();
		
		Helpers::debugAssert(!mode.autoPause, "Enabled CDROM auto pause\n");
		Helpers::debugAssert(!mode.report, "Enabled CDROM report\n");
		Helpers::debugAssert(!mode.xaFilter, "Enabled CDROM xa-filter\n");
		Helpers::debugAssert(!mode.ignoreBit, "Enabled CDROM ignore bit\n");
		Helpers::debugAssert(!mode.sectorSize, "Enabled CDROM 0x924 byte sectors\n");
		Helpers::debugAssert(!mode.xaAdpcm, "Enabled CDROM xa-adpcm\n");

		response.push(statusCode.raw);
		scheduler->push(&int3, scheduler->time + int3Delay, this);

		log("Setmode\n");
		break;
	}

	case CDROMCommands::Demute: {
		response.push(statusCode.raw);
		secondResponse.push(statusCode.raw);

		scheduler->push(&int3, scheduler->time + int3Delay, this);

		log("Demute\n");
		break;
	}
	
	case CDROMCommands::SeekL: {
		// TODO: SeekL/P should stop ongoing reads

		// First response doesn't have seeking bit set
		response.push(statusCode.raw);

		statusCode.seeking = true;
		secondResponse.push(statusCode.raw);

		scheduler->push(&int3, scheduler->time + int3Delay, this);
		scheduler->push(&int2, scheduler->time + int3Delay + int2Delay, this);

		// Seek time is currently stubbed to 150k cycles
		scheduler->push(&stopSeeking, scheduler->time + seekTime, this);

		log("SeekL\n");
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
			Helpers::panic("[  FATAL  ] Unimplemented CDROM test subfunc 0x%02x\n", subFunc);
		}
		
		scheduler->push(&int3, scheduler->time + int3Delay, this);

		log("Test\n");
		break;
	}

	case CDROMCommands::GetID: {
		response.push(statusCode.raw);

		// No Disk
		/*secondResponse.push(0x08);
		secondResponse.push(0x40);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		INT5 !!!
		*/

		// Modchip:Audio/Mode1
		secondResponse.push(0x02);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		secondResponse.push(0x00);
		secondResponse.push('C');
		secondResponse.push('H');
		secondResponse.push('S');
		secondResponse.push('T');

		scheduler->push(&int3, scheduler->time + int3Delay, this);
		scheduler->push(&int2, scheduler->time + int3Delay + getIDDelay, this);

		log("GetID\n");
		break;
	}

	default:
		Helpers::panic("[  FATAL  ] Unimplemented CDROM command 0x%02x\n", data);
	}

	Helpers::debugAssert(params.size() == 0, "[  FATAL  ] CDROM command did not use all parameters");
	
	statusReg.rslrrdy = 1;	// Response fifo not empty
	statusReg.prmempt = 1;	// Parameter fifo empty
	statusReg.prmwrdy = 1;	// Parameter fifo not full
}

bool CDROM::shouldFireIRQ() {
	return intFlag & intEnable;
}

void CDROM::int2(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[  FATAL  ] CDROM INT2 was fired before previous INT%d was acknowledged in interrupt flag\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 2;

	// Second response
	if (cdrom->secondResponse.size()) {
		Helpers::debugAssert(!cdrom->response.size(), "[  FATAL  ] CDROM INT2 before first response was read (probably not supposed to happen...?");
		cdrom->response = cdrom->secondResponse;
		cdrom->statusReg.rslrrdy = 1;	// Response fifo not empty

		while (cdrom->secondResponse.size()) cdrom->secondResponse.pop();
		log("--INT2\n");
	}
	else
		log("INT2\n");
}

void CDROM::int3(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[  FATAL  ] CDROM INT3 was fired before previous INT%d was acknowledged in interrupt flag\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 3;

	log("-INT3\n");
}

void CDROM::int5(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[  FATAL  ] CDROM INT5 was fired before previous INT%d was acknowledged in interrupt flag\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 5;

	// Second response
	if (cdrom->secondResponse.size()) {
		Helpers::debugAssert(!cdrom->response.size(), "[  FATAL  ] CDROM INT5 before first response was read (probably not supposed to happen...?");
		cdrom->response = cdrom->secondResponse;
		cdrom->statusReg.rslrrdy = 1;	// Response fifo not empty

		while (cdrom->secondResponse.size()) cdrom->secondResponse.pop();
		log("--INT5\n");
	}
	else
		log("INT5\n");
}

void CDROM::stopSeeking(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	cdrom->statusCode.seeking = false;
	log("Seek ended\n");
}

void CDROM::beginReading() {
	statusCode.reading = true;
	log("Begin reading\n");
}

void CDROM::stopReading() {
	statusCode.reading = false;
	log("Stop reading\n");
	scheduler->deleteAllEventsOfName("cdRead");
}

void CDROM::cdRead(void* classptr) {
	CDROM* cdrom = (CDROM*)classptr;
	Helpers::debugAssert((cdrom->intFlag & 7) == 0, "[  FATAL  ] CDROM INT1 was fired before previous INT%d was acknowledged in interrupt flag\n", cdrom->intFlag & 3);
	cdrom->intFlag |= 1;

	cdrom->sector.clear();
	cdrom->sector.resize(sectorSize);
	std::fseek(cdrom->cd, (cdrom->seekLoc - 150) * sectorSize, SEEK_SET);
	std::fread(cdrom->sector.data(), 1, sectorSize, cdrom->cd);
	cdrom->statusReg.drqsts = 1;

	log("Read sector %d\n", cdrom->seekLoc);
	cdrom->seekLoc++;

	if (cdrom->mode.sectorSize)
		cdrom->sectorCur = 0x0C;
	else
		cdrom->sectorCur = 0x18;

	cdrom->scheduler->push(&cdrom->cdRead, cdrom->scheduler->time + (!cdrom->mode.doubleSpeed ? readTime : readTimeDoubleSpeed), classptr, "cdRead");
}

u32 CDROM::readSectorWord() {
	if ((sectorCur + 3) >= sector.size()) Helpers::panic("[  FATAL  ] CDROM sector read out of bounds\n");
	
	u32 word;
	std::memcpy(&word, &sector[sectorCur], sizeof(u32));
	sectorCur += sizeof(u32);

	if (sectorCur == sectorSize) statusReg.drqsts = 0;

	return word;
}

void CDROM::pushParam(u8 data) {
	params.push(data);
	Helpers::debugAssert(params.size() <= 16, "[  FATAL  ] Wrote more than 16 bytes to CDROM parameter fifo");
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

u8 CDROM::readIE() {
	return intEnable;
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