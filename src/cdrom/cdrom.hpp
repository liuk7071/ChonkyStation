#pragma once

#include <helpers.hpp>
#include <logger.hpp>
#include <BitField.hpp>
#include <queue>
#include <scheduler.hpp>


constexpr u64 cpuSpeed = 33868800;
constexpr u64 readTime = cpuSpeed / 75;
constexpr u64 readTimeDoubleSpeed = readTime / 2;

// I don't know if these are ok?????
constexpr u64 int3Delay = cpuSpeed / 15000;
constexpr u64 int2Delay = int3Delay * 2;
constexpr u64 getIDDelay = 33868;

constexpr u64 seekTime = 75000;	// Currently stubbed seeking time to this for all seeks

constexpr u64 sectorSize = 0x930;

class CDROM {
public:
	CDROM(const fs::path& cdPath, Scheduler* scheduler);
	Scheduler* scheduler;

	FILE* cd;

	void executeCommand(u8 data);
	
	// INTs
	bool shouldFireIRQ();

	static void int2(void* classptr);
	static void int3(void* classptr);
	static void int5(void* classptr);
	
	static void stopSeeking(void* classptr);

	void beginReading();
	void stopReading();
	static void cdRead(void* classptr);

	void pushParam(u8 data);

	u8 readStatus();
	void writeStatus(u8 data);
	
	u8 readIE();
	void writeIE(u8 data);
	u8 readIF();
	void writeIF(u8 data);

	u8 getIndex();

	u8 getResponseByte();	// This one is public so it can be called from the memory read handlers
	u32 readSectorWord();
private:
	std::vector<u8> sector;
	u64 sectorCur = 0;

	u8 intEnable = 0;
	u8 intFlag = 0;

	std::queue<u8> params;
	u8 getParamByte();

	u32 seekLoc = 0;
	u8 bcdToDec(u8 n) { return n - 6 * (n >> 4); }

	union {
		u8 raw = 0;
		BitField<0, 1, u8> cdda;
		BitField<1, 1, u8> autoPause;
		BitField<2, 1, u8> report;
		BitField<3, 1, u8> xaFilter;
		BitField<4, 1, u8> ignoreBit;
		BitField<5, 1, u8> sectorSize;
		BitField<6, 1, u8> xaAdpcm;
		BitField<7, 1, u8> doubleSpeed;
	} mode;

	std::queue<u8> response;
	std::queue<u8> secondResponse;

	union {
		u8 raw = 0;
		BitField<0, 2, u8> index;
		BitField<2, 1, u8> adpbusy;
		BitField<3, 1, u8> prmempt;
		BitField<4, 1, u8> prmwrdy;
		BitField<5, 1, u8> rslrrdy;
		BitField<6, 1, u8> drqsts;
		BitField<7, 1, u8> busysts;
	} statusReg;

	union {
		u8 raw = 0;
		BitField<0, 1, u8> error;
		BitField<1, 1, u8> motor;
		BitField<2, 1, u8> seekError;
		BitField<3, 1, u8> idError;
		BitField<4, 1, u8> shellOpen;
		BitField<5, 1, u8> reading;
		BitField<6, 1, u8> seeking;
		BitField<7, 1, u8> playing;
	} statusCode;
};

namespace CDROMCommands {
enum {
	GetStat = 0x01,
	SetLoc  = 0x02,
	ReadN	= 0x06,
	Pause	= 0x09,
	Init	= 0x0A,
	Setmode = 0x0E,
	Demute	= 0x0C,
	SeekL	= 0x15,
	Test	= 0x19,
	GetID   = 0x1A
};
}	// End namespace CDROMCommands