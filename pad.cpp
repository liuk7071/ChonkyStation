#include "pad.h"

void pad::WriteTXDATA(uint8_t data) {
	switch (data) {
	case 0:
	case 1: {
		bytes_read = 0;
		read_response = true;
		rx_data_fifo[0] = 0xff;
		break;
	}
	case 0x42:
	case 0x43: {
		bytes_read = 0;
		read_response = true;
		if ((joy_ctrl & 0x2002) == 2) {
			rx_data_fifo[0] = 0x41;
			rx_data_fifo[1] = 0x5A;
			rx_data_fifo[2] = P1buttons & 0xff;
			rx_data_fifo[3] = (P1buttons >> 8) & 0xff;
		}
		else if ((joy_ctrl & 0x2002) == 0x2002) {
			rx_data_fifo[0] = 0x41;
			rx_data_fifo[1] = 0x5A;
			rx_data_fifo[2] = P2buttons & 0xff;
			rx_data_fifo[3] = (P2buttons >> 8) & 0xff;
		}
		break;
	}
	case 0x03: break;
	case 0x44: read_response = false; break;
	case 0x4d: read_response = false; break;
	case 0xff: break;
	default:
		printf("[PAD] Received unhandled command 0x%x\n", data);
		exit(0);
	}
}

uint8_t pad::ReadRXFIFO() {
	if (read_response) {
		return rx_data_fifo[bytes_read++];
	}
	else return 0;
}