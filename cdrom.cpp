#include "cdrom.h"
#define disk

cdrom::cdrom() {
}

uint8_t cdrom::get_stat() {
	return 0b00000010;	// Pretend that the motor is on
}

void cdrom::push(uint8_t data) {
	printf("[CDROM] Parameter: %xh\n", data);
	fifo[cmd_length] = data;
	cmd_length++;
	status &= ~0b00001000; // Parameter fifo empty bit
}

void cdrom::execute(uint8_t command) {
	switch (command) {

	case 0x01: GetStat(); break;
	case 0x02: SetLoc(); break;
	case 0x0A: init(); break;
	case 0x0E: Setmode(); break;
	case 0x15: SeekL(); break;
	case 0x19: test(); break;
	case 0x1A: GetID(); break;

	default: printf("[CDROM] Unimplemented command: 0x%x\n", command); exit(0);

	}
	return;
}

uint8_t cdrom::read_fifo() {
	uint8_t byte = response_fifo[0];
	if (response_length == 1) {
		status &= ~0b00100000;
		printf("[CDROM] Response fifo empty, new status value: 0x%x\n", status);
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
void cdrom::INT2() {
	interrupt_flag &= ~0b111;
	interrupt_flag |= 0b010;
	return;
}
void cdrom::INT3() {
	printf("a\n");
	interrupt_flag &= ~0b111;
	interrupt_flag |= 0b011;
	return;
}
void cdrom::INT5() {
	interrupt_flag &= ~0b111;
	interrupt_flag |= 0b101;
	return;
}
void cdrom::delayedINT() {
	if (delayed_INT3) {
		delayed_INT3 = false;
		INT3();
	}
}
void cdrom::sendQueuedINT() {
	delay = queued_delay;
	if (queued_INT2) {
		//printf("[CDROM] Sending delayed INT2\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		INT2();
		status |= 0b00100000;
		queued_INT2 = false;
		
	}
	if (queued_INT3) {
		//printf("[CDROM] Sending delayed INT3\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		INT3();
		status |= 0b00100000;
		queued_INT3 = false;

	}
	if (queued_INT5) {
		//printf("[CDROM] Sending delayed INT5\n");
		for (int i = 0; i < 16; i++) {
			response_fifo[i] = queued_fifo[i];
		}
		response_length = queued_response_length;
		INT5();
		status |= 0b00100000;
		queued_INT5 = false;
	}
	return;
}
// commands
void cdrom::test() {
	status |= 0b00001000;
	switch (fifo[0]) {
	case 0x20: { // Get cdrom BIOS date/version (yy,mm,dd,ver)
		printf("[CDROM] test 20h\n");
		response_fifo[0] = 0x94;
		response_fifo[1] = 0x09;
		response_fifo[2] = 0x19;
		response_fifo[3] = 0xC0;
		response_length = 4;
		INT3();
		break;
	}
	default:
		printf("[CDROM] Unhandled test command: %xh\n", fifo[0]);
		exit(0);
	}
	delay = 2;
}
void cdrom::GetStat() {	// Return status byte
	printf("[CDROM] GetStat\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	INT3();
	status |= 0b00100000;
	delay = 40000;
}
void cdrom::Setmode() {	// Set mode 
	status |= 0b00001000;
	printf("[CDROM] Setmode (0x%x)\n", fifo[0]);
	response_fifo[0] = get_stat();
	INT3();
	status |= 0b00100000;
	delay = 2;
}
void cdrom::GetID() { // Disk info
	printf("[CDROM] GetID\n");
#ifdef disk
	response_fifo[0] = get_stat();
	response_length = 1;
	INT3();
	queued_fifo[0] = 0x02;
	queued_fifo[1] = 0;
	queued_fifo[2] = 0;
	queued_fifo[3] = 0;
	queued_fifo[4] = 0x53;
	queued_fifo[5] = 0x43;
	queued_fifo[6] = 0x45;
	queued_fifo[7] = 0x41;
	queued_response_length = 8;
	queued_INT2 = true;
#else
	response_fifo[0] = get_stat();
	response_length = 1;
	INT3();
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
#endif
	delay = 40000;
	queued_delay = 50000;
	status |= 0b00100000;
}
void cdrom::SetLoc() { // Sets the seek target
	status |= 0b00001000;
	printf("[CDROM] SetLoc\n");
	amm = fifo[0];		// 
	ass = fifo[1];		//	I don't really know what these do yet so I'm just storing them in 3 variables. I'm guessing I'll need them later to know where to read from
	asect = fifo[2];	// 
	response_fifo[0] = get_stat();
	response_length = 1;
	INT3();
	status |= 0b00100000;
	delay = 10000;
}
void cdrom::SeekL() {	// Seek to the seek target (SetLoc)
	status |= 0b00001000;
	printf("[CDROM] SeekL\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	delayed_INT3 = true;
	queued_fifo[0] = get_stat();
	queued_response_length = 1;
	queued_INT2 = true;
	status |= 0b00100000;
	delay = 100000;
	queued_delay = 100000;
}
void cdrom::init() {
	printf("[CDROM] Init\n");
	response_fifo[0] = get_stat();
	response_length = 1;
	INT3();
	delay = 40000;
	queued_fifo[0] = 0b00000010;
	queued_response_length = 1;
	queued_INT2 = true;
	queued_delay = 50000;
	status |= 0b00100000;
}