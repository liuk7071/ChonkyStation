#include "diff.hpp"


using namespace Test;

Diff::Diff(const fs::path& biosPath, const fs::path& cdPath) : p1(biosPath, cdPath), p2(biosPath, cdPath) {
	p1.switchCpuBackend(Cpu::Backend::Interpreter);
	p2.switchCpuBackend(Cpu::Backend::OldInterpreter);
}

void Diff::doTest() {
	u32* gprs1 = p1.getGPRS();
	u32* gprs2 = p2.getGPRS();

	while (true) {
		while (!p1.getVBLANKAndClear()) {
			p1.step();
			p2.step();

			for (int i = 0; i < 32; i++) {
				if (gprs1[i] != gprs2[i]) {
					printf("r%d | 0x%08x != 0x%08x\n", i, gprs1[i], gprs2[i]);
					exit(0);
				}
			}

			if (p1.getPC() != p2.getPC()) {
				printf("pc | 0x%08x != 0x%08x\n", p1.getPC(), p2.getPC());
				
				Helpers::dump("ramp1.bin", p1.getRAM(), 2_MB);
				Helpers::dump("ramp2.bin", p2.getRAM(), 2_MB);
				exit(0);
			}
		}
		p1.VBLANK();
		p2.VBLANK();
	}
}