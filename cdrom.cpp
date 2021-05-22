#include "cdrom.h"
cdrom::cdrom() {
}
void cdrom::push(uint8_t data) {
	printf("[CDROM] Parameter: %xh\n", data);
	fifo[cmd_length] = data;
	cmd_length++;
}

void cdrom::execute(uint8_t command) {
	switch (command) {

	case 0x01: GetStat(); break;
	case 0x19: test(); break;

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
void cdrom::INT3() {
	interrupt_flag &= ~0b111;
	interrupt_flag |= 0b011;
	return;
}
// commands
void cdrom::test() {
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
}
void cdrom::GetStat() {
	printf("[CDROM] GetStat\n");
	response_fifo[0] = 0;
	response_length = 1;
	INT3();
	status |= 0b00100000;
}