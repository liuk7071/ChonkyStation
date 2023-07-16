#include <intc.hpp>


void INTC::writeImask(u16 data) {
	imask = data;
}

void INTC::writeIstat(u16 data) {
	istat &= data;
}

u16 INTC::readImask() {
	return imask;
}

u16 INTC::readIstat() {
	return istat;
}