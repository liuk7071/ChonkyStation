#pragma once

#include <helpers.hpp>
#include <BitField.hpp>
#include <queue>


class CDROM {
public:
	CDROM();

	void executeCommand(u8 data);
	void pushParam(u8 data);

	u8 readStatus();
	void writeStatus(u8 data);
	
	void writeIE(u8 data);
	void writeIF(u8 data);

	u8 getIndex();
private:
	std::queue<u8> params;

	union {
		u8 raw = 0;
		BitField<0, 2, u8> index;
		BitField<2, 1, u8> adpbusy;
		BitField<3, 1, u8> prmempt;
		BitField<4, 1, u8> prmwrdy;
		BitField<5, 1, u8> rslrrdy;
		BitField<6, 1, u8> drqsts;
		BitField<7, 1, u8> busysts;
	} status;
};