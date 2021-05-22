#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
class cdrom
{
public:
	cdrom();
	bool irq;
public: // fifo
	uint8_t fifo[16];
	int cmd_length;
	int response_length = 0; // last command's response length
	void push(uint8_t data);
public:
	void execute(uint8_t command);
	uint8_t read_fifo();
	uint8_t status = 0b00011000;
	uint8_t request;
	uint8_t interrupt_enable;
	uint8_t interrupt_flag;
	uint8_t response_fifo[16];
	void INT3();
	// commands
	void test();
	void GetStat();
};

