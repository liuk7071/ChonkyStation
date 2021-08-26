#include "CD.h"
CD::CD(const char* directory) {
	iso = fopen(directory, "r");
}

uint8_t CD::ReadDataByte() {
	if (buff_left == CDXA_DATA_SIZE)
		printf("[All data has been read]\n");
	return DataBuffer[buff_left++];
}

void CD::read(uint32_t loc) {
	buff_left = CDXA_DATA_SIZE;
	printf("[CD] Read sector 0x%x\n", loc);
	fseek(iso, loc, SEEK_SET);
	fread(SectorBuffer, sizeof(uint8_t), SECTOR_SIZE, iso);
	memcpy(DataBuffer, &SectorBuffer[0x18], 0x800);
}