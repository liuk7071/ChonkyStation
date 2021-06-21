#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
class controller
{
public:
	controller();
public:
	void exec();
public:
	uint16_t id = 0x5A41; // Digital controller
	uint16_t buttons = 0b1011111111111111;	// All released
	uint16_t right_joy = 0;	// Right and left joysticks (analog pads only)
	uint16_t left_joy = 0;	

	uint16_t joy_ctrl = 0b0001000000000001;
	uint16_t joy_baud = 0;
	uint16_t joy_mode = 0;
	uint16_t joy_stat = 0;
	uint16_t joy_tx_data = 0;
	uint16_t joy_rx_data = 0;
};

