#include "Bus.h"
Bus::Bus() {
	CDROM.connectMem(&mem);
}

Bus::~Bus() {

}

