#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <cstdarg>
#include "CD.h"
#include "scheduler.h"

#define INT1_DELAY ((33868800 / 75) / (DoubleSpeed ? 2 : 1))

class cdrom
{
public:
	cdrom();
	scheduler Scheduler;
	bool irq = false;
public: // fifo
	uint8_t fifo[16];
	uint8_t queued_fifo[16]; // queued response
	int cmd_length = 0;
	int response_length = 0; // last command's response length
	int queued_response_length = 0; // queued response's length
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

	bool reading = false;
	bool cancel_int1 = false;

	// commands
	void test();
	void GetStat();
	void GetTN();
	void GetTD();
	void GetID();
	void SetLoc();
	void Play();
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
	void GetLocP();

	// setmode
	bool DoubleSpeed = false;
	bool xa_adpcm = false;
	bool WholeSector = false;
	bool CDDA = false;
	
	CD cd = CD();
};
