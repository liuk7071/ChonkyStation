#include <interrupt.hpp>


void Interrupt::writeImask(u16 data) {
	imask = data;
}

void Interrupt::writeIstat(u16 data) {
	istat &= data;
}

u16 Interrupt::readImask() {
	return imask;
}

u16 Interrupt::readIstat() {
	return istat;
}