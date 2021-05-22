#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
class cdrom
{
public:
	cdrom();
public: // fifo
	uint8_t fifo[16];
	int cmd_length = 0;
	void push(uint8_t data);
public:
	void execute(uint8_t command);

	// commands
	void test();
};

