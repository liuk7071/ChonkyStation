#include "CD.h"
CD::CD(const char* directory) {
	iso = fopen(directory, "r");
}

uint8_t CD::PopSectorByte() {
	uint8_t byte = DataBuffer[0];
	if (buff_left == 1) {
		printf("[CD] Whole sector buffer has been read\n");
		DataBuffer[0] = 0;
		return byte;
	}
	int i;
	uint8_t shifted = 0x00;
	uint8_t overflow = (0xF0 & DataBuffer[0]) >> 4;

	for (int x = 1; x <= 2; x++) {
		for (i = (16 - 1); i >= 0; i--)
		{
			shifted = (DataBuffer[i] << 4) | overflow;
			overflow = (0xF0 & DataBuffer[i]) >> 4;
			DataBuffer[i] = shifted;
		}
	}
	buff_left--;
	return byte;
}

void CD::read(uint32_t loc) {
	buff_left = CDXA_DATA_SIZE;
	printf("[CD] Read sector %d\n", loc);
	fseek(iso, loc, SEEK_SET);
	fread(SectorBuffer, sizeof(uint8_t), BUFFER_SIZE, iso);
	memcpy(&DataBuffer, &SectorBuffer[0x18], 0x914);
	//for (auto& i : DataBuffer) {
	//	printf("%x\n", DataBuffer[i]);
	//	
	//}
}