#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <cstdarg>
#include "CD.h"
#include "scheduler.h"

class cdrom
{
public:
	cdrom();
	scheduler Scheduler;
	bool irq = false;
	int delay = 1; // the INT delay in cycles
public: // fifo
	uint8_t fifo[16];
	uint8_t queued_fifo[16]; // queued response
	int cmd_length = 0;
	int response_length = 0; // last command's response length
	int queued_response_length = 0; // queued response's length
	int queued_delay = 1; // queued INT's delay in cycles
	uint8_t get_stat(); // get status byte
	void push(uint8_t data);
public:
	void execute(uint8_t command);
	uint8_t read_fifo();
	uint8_t status = 0b00011000;
	uint8_t request = 0;
	uint8_t interrupt_enable = 0;
	uint8_t interrupt_flag = 0;
	uint8_t response_fifo[16];
	
	uint8_t mm = 0, ss = 0, ff = 0;
	uint32_t seekloc = 0;	// Set by SetLoc
	uint8_t bcd_dec(uint8_t val);	// Convert BCD to decimal

	static void queuedRead(void* dataptr);
	static void INT1(void* dataptr);
	static void INT2(void* dataptr);
	static void INT3(void* dataptr);
	static void INT5(void* dataptr);
	bool INT = false; // true if there are INTs to acknowledge
	bool queued_INT1 = false;
	bool queued_INT2 = false;
	bool queued_INT3 = false;
	bool queued_INT5 = false; // true if an INT5 is queued
	bool queued_read = false; // true if a read is queued. ReadN/S should keep reading sectors until a Pause command is sent
	bool delayed_INT1 = false;
	bool delayed_INT2 = false;
	bool delayed_INT3 = false; // INT3 should be delayed
	bool reading = false;
	void delayedINT();
	//void sendQueuedINT();
	// commands
	void test();
	void GetStat();
	void GetTN();
	void GetTD();
	void GetID();
	void SetLoc();
	void SeekL();
	void ReadN();
	void ReadS();
	void Stop();
	void Pause();
	void init();
	void Demute();
	void SetFilter();
	void Setmode();
	void GetLocL();

	// setmode
	bool DoubleSpeed = false;
	bool xa_adpcm = false;
	bool WholeSector = false;
	bool CDDA = false;
	
	CD cd = CD();
};
