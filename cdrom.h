#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
class cdrom
{
public:
	cdrom();
	bool disk = false; // disk inserted
	bool irq;
	int delay = 1; // the INT delay in cycles
public: // fifo
	uint8_t fifo[16];
	uint8_t queued_fifo[16]; // queued response
	int cmd_length;
	int response_length = 0; // last command's response length
	int queued_response_length = 0; // queued response's length
	int queued_delay = 1; // queued INT's delay in cycles
	uint8_t get_stat(); // get status byte
	void push(uint8_t data);
public:
	void execute(uint8_t command);
	uint8_t read_fifo();
	uint8_t status = 0b00011000;
	uint8_t request;
	uint8_t interrupt_enable;
	uint8_t interrupt_flag;
	uint8_t response_fifo[16];
	
	uint8_t amm;
	uint8_t ass;
	uint8_t asect;

	void INT2();
	void INT3();
	void INT5();
	bool INT = false; // true if there are INTs to acknowledge
	bool queued_INT2 = false;
	bool queued_INT5 = false; // true if an INT5 is queued
	void sendQueuedINT();
	// commands
	void test();
	void GetStat();
	void GetID();
	void SetLoc();
	void init();
};
