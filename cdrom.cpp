#include "cdrom.h"
#define disk
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
	uint8_t stat = 0b10;
	if (reading)
		stat |= (1 << 5);
	return stat;	// Pretend that the motor is always on
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
	case 0x06: ReadN(); break;
	case 0x09: Pause(); break;
	case 0x0A: init(); break;
	case 0x0E: Setmode(); break;
	case 0x15: SeekL(); break;
	case 0x19: test(); break;
	case 0x1A: GetID(); break;

	default: debug_log("[CDROM] Unimplemented command: 0x%x\n", command); exit(0);

	}
	cmd_length = 0;
	for (auto& i : fifo)
		fifo[i] = 0;
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
		cdromptr->seekloc++;
		cdromptr->cd.read(cdromptr->seekloc);
		cdromptr->queued_fifo[0] = cdromptr->get_stat();
		cdromptr->queued_response_length = 1;

		cdromptr->Scheduler.push(&INT1, cdromptr->Scheduler.time + 100000, cdromptr);
	}
}
void cdrom::INT1(void* dataptr) {
	printf("[IRQ] INT1 dispatched\n");
	cdrom* cdromptr = (cdrom*)(dataptr);
	cdromptr->interrupt_flag &= ~0b111;
	cdromptr->interrupt_flag |= 0b001;

	for (int i = 0; i < 16; i++) {
		cdromptr->response_fifo[i] = cdromptr->queued_fifo[i];
	}
	cdromptr->response_length = cdromptr->queued_response_length;
	cdromptr->status |= 0b00100000;

	cdromptr->Scheduler.push(&queuedRead, cdromptr->Scheduler.time + 300000, cdromptr);
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
/*void cdrom::queuedRead() {
	if (queued_read) {
		if (reading) {
			seekloc += 0x930;
			cd.read(seekloc);
			response_fifo[0] = get_stat();
			response_length = 1;
			//INT1();
			delay = 2;
		}
		queued_read = false;
	}
}*/
/*void cdrom::sendQueuedINT() {
	delay = queued_delay;
	if (queued_INT1) {
		//debug_log("[CDROM] Sending delayed INT1\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		delayed_INT1 = true;
		status |= 0b00100000;
		queued_INT1 = false;

	}
	if (queued_INT2) {
		//debug_log("[CDROM] Sending delayed INT2\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		delayed_INT2 = true;
		status |= 0b00100000;
		queued_INT2 = false;
	}
	if (queued_INT3) {
		//debug_log("[CDROM] Sending delayed INT3\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		//INT3();
		status |= 0b00100000;
		queued_INT3 = false;
	}
	if (queued_INT5) {
		//debug_log("[CDROM] Sending delayed INT5\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		//INT5();
		status |= 0b00100000;
		queued_INT5 = false;
	}
	return;
}*/
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
	response_fifo[0] = get_stat();
	response_length = 1;
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
	if (WholeSector) debug_log("WholeSector\n");
	if (CDDA) debug_log("CDDA\n");
	response_fifo[0] = get_stat();
	//INT3();
	status |= 0b00100000;
	Scheduler.push(&INT3, Scheduler.time + 4000, this);
	//delay = 2;
}
void cdrom::GetID() { // Disk info
	debug_log("[CDROM] GetID\n");
#ifdef disk
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
	std::memcpy(&queued_fifo[4], "CHNK", 4);
	queued_response_length = 8;
	queued_INT2 = true;
	Scheduler.push(&INT2, Scheduler.time + 50000, this);
#else
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
#endif
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
void cdrom::SeekL() {	// Seek to the seek target (SetLoc)
	status |= 0b00001000;
	debug_log("[CDROM] SeekL\n");
	response_fifo[0] = get_stat();
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
	cd.read(seekloc);
	response_fifo[0] = get_stat();
	response_length = 1;
	//INT3();
	//delay = 2;

	queued_fifo[0] = get_stat();
	queued_response_length = 1;
	//queued_INT1 = true;

	Scheduler.push(&INT3, Scheduler.time + 4, this);
	Scheduler.push(&INT1, Scheduler.time + 400000, this);
	//queued_delay = 33868800 / 75;
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
	Scheduler.push(&INT2, Scheduler.time + 45000, this);
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