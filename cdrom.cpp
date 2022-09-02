#include "cdrom.h"
#define log

void debug_log(const char* fmt, ...) {
#ifdef log
	std::va_list args;
	va_start(args, fmt);
	std::vprintf(fmt, args);
	va_end(args);
#endif
}
cdrom::cdrom() {
}

uint8_t cdrom::get_stat() {
	uint8_t stat = 0b10;	// Pretend that the motor is always on
	if (reading)
		stat |= (1 << 5);
	return stat;
}

uint8_t cdrom::bcd_dec(uint8_t val) {
	return val - 6 * (val >> 4);
}

void cdrom::push(uint8_t data) {
	debug_log("[CDROM] Parameter: %xh\n", data);
	fifo[cmd_length] = data;
	cmd_length++;
	status &= ~0b00001000; // Parameter fifo empty bit
}

void cdrom::execute(uint8_t command) {
	switch (command) {

	case 0x01: GetStat(); break;
	case 0x02: SetLoc(); break;
	case 0x03: Play(); break;
	case 0x06: ReadN(); break;
	case 0x08: Stop(); break;
	case 0x09: Pause(); break;
	case 0x0A: init(); break;
	case 0x0C: Demute(); break;
	case 0x0D: SetFilter(); break;
	case 0x0E: Setmode(); break;
	case 0x10: GetLocL(); break;
	case 0x11: GetLocP(); break;
	case 0x13: GetTN(); break;
	case 0x14: GetTD(); break;
	case 0x15: SeekL(); break;
	case 0x19: test(); break;
	case 0x1A: GetID(); break;
	case 0x1B: ReadS(); break;

	default: debug_log("[CDROM] Unimplemented command: 0x%x\n", command); exit(0);

	}
	cmd_length = 0;
	for (int i = 0; i < 16; i++) fifo[i] = 0;
	return;
}

uint8_t cdrom::read_fifo() {
	uint8_t byte = response_fifo[0];
	if (response_length == 1) {
		status &= ~0b00100000;	// Clear response fifo empty bit (means it's empty)
		debug_log("[CDROM] Response fifo empty, new status value: 0x%x\n", status);
		response_fifo[0] = 0;
		return byte;
	}
	int i;
	uint8_t shifted = 0x00;
	uint8_t overflow = (0xF0 & response_fifo[0]) >> 4;

	for (int x = 1; x <= 2; x++) {
		for (i = (16 - 1); i >= 0; i--)
		{
			shifted = (response_fifo[i] << 4) | overflow;
			overflow = (0xF0 & response_fifo[i]) >> 4;
			response_fifo[i] = shifted;
		}
	}
	response_length--;
	return byte;
}

void cdrom::queuedRead(void* dataptr) {
	cdrom* cdromptr = (cdrom*)(dataptr);
	if (cdromptr->reading) {
		//cdromptr->seekloc++;
		cdromptr->status |= 0b10000000; // DRQSTS
		cdromptr->queued_fifo[0] = cdromptr->get_stat();
		cdromptr->queued_response_length = 1;

		cdromptr->Scheduler.push(&INT1, cdromptr->Scheduler.time + 5000, cdromptr);
	}
}
void cdrom::INT1(void* dataptr) {
	cdrom* cdromptr = (cdrom*)(dataptr);
	if (cdromptr->reading) {
		cdromptr->cd.read(cdromptr->seekloc++);

		bool audio_sector = (cdromptr->cd.SectorBuffer[0x12] & 4);
		bool realtime = (cdromptr->cd.SectorBuffer[0x12] & 64);
		if (!cdromptr->xa_adpcm || !(audio_sector && realtime)) {
			printf("[IRQ] INT1 dispatched\n");
			cdromptr->interrupt_flag &= ~0b111;
			cdromptr->interrupt_flag |= 0b001;

			for (int i = 0; i < 16; i++) {
				cdromptr->response_fifo[i] = cdromptr->queued_fifo[i];
			}
			cdromptr->response_length = cdromptr->queued_response_length;
			cdromptr->status |= 0b00100000;
			cdromptr->cd.bytes_read = 0;
		}
		cdromptr->Scheduler.push(&queuedRead, cdromptr->Scheduler.time + ((33868800 / 75) / (cdromptr->DoubleSpeed ? 2 : 1)), cdromptr);
	}
	return;
}
void cdrom::INT2(void* dataptr) {
	printf("[IRQ] INT2 dispatched\n");
	cdrom* cdromptr = (cdrom*)(dataptr);
	cdromptr->interrupt_flag &= ~0b111;
	cdromptr->interrupt_flag |= 0b010;

	for (int i = 0; i < 16; i++) {
		cdromptr->response_fifo[i] = cdromptr->queued_fifo[i];
	}
	cdromptr->response_length = cdromptr->queued_response_length;
	cdromptr->status |= 0b00100000;
	return;
}
void cdrom::INT3(void* dataptr) {
	printf("[IRQ] INT3 dispatched\n");
	cdrom* cdromptr = (cdrom*)(dataptr);
	cdromptr->interrupt_flag &= ~0b111;
	cdromptr->interrupt_flag |= 0b011;
	return;
}
void cdrom::INT5(void* dataptr) {
	printf("[IRQ] INT5 dispatched\n");
	cdrom* cdromptr = (cdrom*)(dataptr);
	cdromptr->interrupt_flag &= ~0b111;
	cdromptr->interrupt_flag |= 0b101;

	for (int i = 0; i < 16; i++) {
		cdromptr->response_fifo[i] = cdromptr->queued_fifo[i];
	}
	cdromptr->response_length = cdromptr->queued_response_length;
	cdromptr->status |= 0b00100000;
	return;
}
void cdrom::delayedINT() {
	if (delayed_INT1) {
		delayed_INT1 = false;
		//INT1();
	}
	if (delayed_INT2) {
		delayed_INT2 = false;
		//INT2();
	}
	if (delayed_INT3) {
		delayed_INT3 = false;
		//INT3();
	}
}

// commands
void cdrom::test() {
	status |= 0b00001000;
	switch (fifo[0]) {
	case 0x20: { // Get cdrom BIOS date/version (yy,mm,dd,ver)
		debug_log("[CDROM] test 20h\n");
		response_fifo[0] = 0x94;
		response_fifo[1] = 0x09;
		response_fifo[2] = 0x19;
		response_fifo[3] = 0xC0;
		response_length = 4;
		Scheduler.push(&INT3, Scheduler.time + 40, this);
		break;
	}
	default:
		debug_log("[CDROM] Unhandled test command: %xh\n", fifo[0]);
		exit(0);
	}
}
void cdrom::GetStat() {	// Return status byte
	debug_log("[CDROM] GetStat\n");
	//response_fifo[0] = get_stat();
	response_fifo[0] = rand() % 0xff;
	response_fifo[0] &= ~0b11101;
	response_length = 1;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 25000, this);
}
void cdrom::GetTN() { // Get first track number, and last track number in the TOC of the current Session.
	debug_log("[CDROM] GetTN (stubbed)\n");
	response_fifo[0] = get_stat();
	response_fifo[1] = 0x00;
	response_fifo[2] = 0x00;
	response_length = 3;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 15000, this);
}
void cdrom::GetTD() {
	debug_log("[CDROM] GetTD (stubbed)\n");
	//response_fifo[0] = get_stat();
	response_fifo[0] = 0x00;
	response_fifo[1] = 0x00;
	response_fifo[2] = 0x00;
	response_length = 3;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 15000, this);
}
void cdrom::Setmode() {	// Set mode 
	status |= 0b00001000;
	debug_log("[CDROM] Setmode (0x%x)\n", fifo[0]);
	DoubleSpeed = (fifo[0] & 0b10000000) >> 7;
	xa_adpcm = (fifo[0] & 0b01000000) >> 6;
	WholeSector = (fifo[0] & 0b00100000) >> 5;
	CDDA = fifo[0] & 1;
	if (DoubleSpeed) debug_log("DoubleSpeed\n");
	if (xa_adpcm) debug_log("XA-ADPCM\n");
	if (WholeSector) {
		printf("WholeSector\n");
	}
	cd.WholeSector = WholeSector;
	if (CDDA) { debug_log("CDDA\n"); 
		//exit(1); 
	}
	response_fifo[0] = get_stat();
	//INT3();
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 4000, this);
	//delay = 2;
}
void cdrom::GetLocL() {
	debug_log("[CDROM] GetLocL\n");
	response_fifo[0] = cd.SectorBuffer[0xc + 0];
	response_fifo[1] = cd.SectorBuffer[0xc + 1];
	response_fifo[2] = cd.SectorBuffer[0xc + 2];
	response_fifo[3] = cd.SectorBuffer[0xc + 3];
	response_fifo[4] = cd.SectorBuffer[0x10 + 0];
	response_fifo[5] = cd.SectorBuffer[0x10 + 1];
	response_fifo[6] = cd.SectorBuffer[0x10 + 2];
	response_fifo[7] = cd.SectorBuffer[0x10 + 3];
	response_length = 8;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 15000, this);
}
void cdrom::GetLocP() {
	debug_log("[CDROM] GetLocP (stubbed)\n");
	response_length = 8;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 15000, this);
}
void cdrom::GetID() { // Disk info
	debug_log("[CDROM] GetID\n");
	if (cd.IsCDInserted) {
		response_fifo[0] = get_stat();
		response_length = 1;
		//INT3();
		queued_fifo[0] = 0x02;
		queued_fifo[1] = 0;
		queued_fifo[2] = 0;
		queued_fifo[3] = 0;
		//queued_fifo[4] = 0x53;
		//ueued_fifo[5] = 0x43;
		//queued_fifo[6] = 0x45;
		//queued_fifo[7] = 0x41;
		std::memcpy(&queued_fifo[4], "CHST", 4);
		queued_response_length = 8;
		queued_INT2 = true;
		Scheduler.push(&INT2, Scheduler.time + 50000, this);
	}
	else {
		response_fifo[0] = get_stat();
		response_length = 1;
		//INT3();
		queued_fifo[0] = 0x08;
		queued_fifo[1] = 0x40;
		queued_fifo[2] = 0;
		queued_fifo[3] = 0;
		queued_fifo[4] = 0;
		queued_fifo[5] = 0;
		queued_fifo[6] = 0;
		queued_fifo[7] = 0;
		queued_response_length = 8;
		queued_INT5 = true;
		Scheduler.push(&INT5, Scheduler.time + 50000, this);
	}
	Scheduler.push(&INT3, Scheduler.time + 4, this);

	//delay = 40000;
	//queued_delay = 50000;
	status |= 0b00100000;
}
void cdrom::SetLoc() { // Sets the seek target
	status |= 0b00001000;
	mm = bcd_dec(fifo[0]);
	ss = bcd_dec(fifo[1]);
	ff = bcd_dec(fifo[2]);

	seekloc = ff + (ss * 75) + (mm * 60 * 75);
	printf("[CDROM] SetLoc (%d;%d;%d %d)\n", mm, ss, ff, seekloc);
	response_fifo[0] = get_stat();
	response_length = 1;

	Scheduler.push(&INT3, Scheduler.time + 5000, this);
	status |= 0b00100000;
}
void cdrom::Play() {
	status |= 0b00001000;
	mm = bcd_dec(fifo[0]);
	ss = bcd_dec(fifo[1]);
	ff = bcd_dec(fifo[2]);

	printf("[CDROM] Play (stubbed)\n");
	response_fifo[0] = get_stat();
	response_length = 1;

	Scheduler.push(&INT3, Scheduler.time + 5000, this);
	status |= 0b00100000;
}
void cdrom::SeekL() {	// Seek to the seek target (SetLoc)
	status |= 0b00001000;
	debug_log("[CDROM] SeekL\n");
	response_fifo[0] = 0x42;
	response_length = 1;
	delayed_INT3 = true;
	queued_fifo[0] = get_stat();
	queued_response_length = 1;
	queued_INT2 = true;
	status |= 0b00100000;
	//delay = 75000;
	queued_delay = 1000000;

	Scheduler.push(&INT3, Scheduler.time + 4, this);
	Scheduler.push(&INT2, Scheduler.time + 400000, this);
}
void cdrom::ReadN() {	// Read with retry
	reading = true;
	status |= 0b00001000;	// Set parameter fifo empty bit
	debug_log("[CDROM] ReadN\n");
	cd.bytes_read = 0;
	//cd.read(seekloc);
	response_fifo[0] = get_stat();
	response_length = 1;
	//INT3();
	//delay = 2;

	queued_fifo[0] = 0x22;
	queued_response_length = 1;
	//queued_INT1 = true;

	Scheduler.push(&INT3, Scheduler.time + 4, this);
	Scheduler.push(&INT1, Scheduler.time + ((33868800 / 75) / (DoubleSpeed ? 2 : 1)), this);
	//queued_delay = 33868800 / 75;
	status |= 0b00100000;	// Set response fifo empty bit (means it's full)
}
void cdrom::ReadS() {	// Read without retry
	status |= 0b00001000;	// Set parameter fifo empty bit
	debug_log("[CDROM] ReadS\n");
	cd.bytes_read = 0;
	response_fifo[0] = get_stat();
	reading = true; 
	response_length = 1;

	queued_fifo[0] = 0x22;
	queued_response_length = 1;

	Scheduler.push(&INT3, Scheduler.time + 4, this);
	Scheduler.push(&INT1, Scheduler.time + ((33868800 / 75) / (DoubleSpeed ? 2 : 1)), this);
	status |= 0b00100000;	// Set response fifo empty bit (means it's full)
}
void cdrom::Stop() {
	status |= 0b00001000;	// Set parameter fifo empty bit
	debug_log("[CDROM] Stop\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	reading = false;
	queued_fifo[0] = get_stat() & ~2;
	queued_response_length = 1;
	queued_INT2 = true;

	Scheduler.push(&INT3, Scheduler.time + 400, this);
	Scheduler.push(&INT2, Scheduler.time + 35000, this);
	//queued_delay = 50000;
	status |= 0b00100000;	// Set response fifo empty bit (means it's full)
}
void cdrom::Pause() {
	status |= 0b00001000;	// Set parameter fifo empty bit
	debug_log("[CDROM] Pause\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	//INT3();
	//delay = 2;
	reading = false;	// The first response's status code still has bit 5 set
	//queued_read = false;
	queued_fifo[0] = get_stat();
	queued_response_length = 1;
	queued_INT2 = true;

	Scheduler.push(&INT3, Scheduler.time + 400, this);
	Scheduler.push(&INT2, Scheduler.time + 35000, this);
	//queued_delay = 50000;
	status |= 0b00100000;	// Set response fifo empty bit (means it's full)
}
void cdrom::init() {
	debug_log("[CDROM] Init\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	//INT3();
	delay = 40000;
	queued_fifo[0] = 0b00000010;
	queued_response_length = 1;
	queued_INT2 = true;

	Scheduler.push(&INT3, Scheduler.time + 30000, this);
	Scheduler.push(&INT2, Scheduler.time + 40000, this);
	queued_delay = 50000;
	status |= 0b00100000;
}
void cdrom::Demute() {
	debug_log("[CDROM] Demute\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 25000, this);
}
void cdrom::SetFilter() {
	debug_log("[CDROM] SetFilter\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 25000, this);
}