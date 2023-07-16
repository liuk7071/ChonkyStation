#pragma once

#include <helpers.hpp>


class INTC {
public:
	void writeImask(u16 data);
	void writeIstat(u16 data);

	u16 readImask();
	u16 readIstat();

private:
	u16 istat = 0;
	u16 imask = 0;
};