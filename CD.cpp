#include "CD.h"

CD::CD() {}
void CD::OpenFile(const char* directory) {
	iso = fopen(directory, "rb");
	dir = directory;
	IsCDInserted = true;
}

uint8_t CD::ReadDataByte() {
	if (bytes_read >= ((WholeSector ? (SECTOR_SIZE - 0xc) : CDXA_DATA_SIZE) - 1)) {
		printf("[All data has been read]\n");
		drqsts = 0;
	}
	return SectorBuffer[(WholeSector ? 0x0c : 0x18) + bytes_read++];
}

void CD::read(uint32_t loc) {
	printf("[CD] Read sector %d\n", loc);
	drqsts = 1;
	fseek(iso, (loc - 150) * SECTOR_SIZE, SEEK_SET);
	fread(SectorBuffer, sizeof(uint8_t), SECTOR_SIZE, iso);
}