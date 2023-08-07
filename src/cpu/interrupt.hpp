#pragma once

#include <helpers.hpp>


class Interrupt {
public:
	void writeImask(u16 data);
	void writeIstat(u16 data);

	u16 readImask();
	u16 readIstat();

	enum class InterruptType {
		VBLANK,
		GPU,
		CDROM,
		DMA,
		TMR0, TMR1, TMR2,
		PAD,
		SIO,
		SPU,
		IRQ10	// Dunno what to call this one
	};

	void raiseInterrupt(InterruptType interrupt);
	bool interruptFired();

private:
	u16 istat = 0;
	u16 imask = 0;
};