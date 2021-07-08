#pragma once
#pragma warning(disable : 4996)
#include <iostream>
#include <string>

#define BUFFER_SIZE 0x930
#define CDXA_DATA_SIZE 0x914

class CD
{
public:
	CD(const char* directory);
public:
	void read(uint32_t loc);
	uint8_t PopSectorByte();
public:
	FILE* iso;
	int buff_left = BUFFER_SIZE;
	uint8_t SectorBuffer[BUFFER_SIZE];
	uint8_t DataBuffer[CDXA_DATA_SIZE];
};

