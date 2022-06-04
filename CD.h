#pragma once
#pragma warning(disable : 4996)
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>

#define SECTOR_SIZE 0x930
#define CDXA_DATA_SIZE 0x800

class CD
{
public:
	CD();
	void OpenFile(const char* directory);
	const char* dir;
	bool IsCDInserted = false;
public:
	void read(uint32_t loc);
	uint8_t ReadDataByte();
	bool WholeSector = false;
public:
	FILE* iso;
	int buff_left = 0;
	uint8_t SectorBuffer[SECTOR_SIZE];
	uint8_t DataBuffer[CDXA_DATA_SIZE];
	uint8_t drqsts = 0;
};

