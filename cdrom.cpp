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
	
	case 0x19: test(); break;
	
	default: printf("[CDROM] Unimplemented command: 0x%x\n", command); exit(0);
	
	}
	return;
}


// commands
void cdrom::test() {
	switch (fifo[0]) {
	case 0x20: { // Get cdrom BIOS date/version (yy,mm,dd,ver)
		printf("[CDROM] test 20h\n");
	}
	default:
		printf("[CDROM] Unhandled test command: %xh\n", fifo[0]);
		exit(0);
	}
}