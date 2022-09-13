#pragma once
#pragma warning(disable : 4996)
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include <filesystem>

#define SECTOR_SIZE 0x930
#define FORM1_DATA_SIZE 0x800
#define FORM2_DATA_SIZE 0x914

class CD
{
public:
	CD();
	void OpenFile(const char* directory);
	const char* dir;
	int file_size = 0;
	bool IsCDInserted = false;
public:
	void read(uint32_t loc);
	uint8_t ReadDataByte();
	bool WholeSector = false;
public:
	FILE* iso;
	int bytes_read = 0;
	uint8_t SectorBuffer[SECTOR_SIZE];
	bool form2 = false;
	uint8_t drqsts = 0;
};

