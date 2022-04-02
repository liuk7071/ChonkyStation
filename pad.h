#pragma once
#include <iostream>
#include <stdint.h>

class pad {
public:
	void WriteTXDATA(uint8_t data);
	uint16_t joy_tx_data = 0;
	uint8_t rx_data_fifo[16];
	int bytes_read = 0;
	bool read_response = false;
	uint8_t ReadRXFIFO();
	uint16_t joy_stat = 0b111;
	uint16_t joy_mode = 0;
	uint16_t joy_ctrl = 0;
	uint16_t joy_baud = 0;

	uint16_t P1buttons = 0xffff;
	uint16_t P2buttons = 0xffff;
};