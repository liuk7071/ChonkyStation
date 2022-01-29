#pragma once
#include "gte.h"

// GTE
class testGTE {
public:
    gte GTE;
    uint32_t command; // To pass to the GTE
    void assignRegisters(uint32_t* cop2c, uint32_t* cop2d) {
        for (int i = 0; i < 32; i++) {
            GTE.cop2c[i] = cop2c[i];
            GTE.cop2d[i] = cop2d[i];
        }
    }
    virtual void checkResult(uint32_t* resCop2c, uint32_t* resCop2d) {
        for (int i = 0; i < 32; i++) {
            if (GTE.cop2c[i] != resCop2c[i]) {
                printf("cnt%d register mismatch: got 0x%08x expected 0x%08x\n", i, GTE.cop2c[i], resCop2c[i]);
            }
        }
        for (int i = 0; i < 32; i++) {
            if (GTE.cop2d[i] != resCop2d[i]) {
                printf("cop2r%d register mismatch: got 0x%08x expected 0x%08x\n", i, GTE.cop2d[i], resCop2d[i]);
            }
        }
    }
};
class testRTPS : public testGTE {
public:
    uint32_t cop2c[32] = {
        0x00000ffb,
        0xffb7ff44,
        0xf9ca0ebc,
        0x063700ad,
        0x00000eb7,
        0x00000000,
        0xfffffeac,
        0x00001700,
        0x00000000,
        0x00000fa0,
        0x0000f060,
        0x0000f060,
        0x00000000,
        0x00000640,
        0x00000640,
        0x00000640,
        0x0bb80fa0,
        0x0fa00fa0,
        0x0fa00bb8,
        0x0bb80fa0,
        0x00000fa0,
        0x00000000,
        0x00000000,
        0x00000000,
        0x01400000,
        0x00f00000,
        0x00000400,
        0xfffffec8,
        0x01400000,
        0x00000155,
        0x00000100,
        0x00000000,
    };
    uint32_t cop2d[32] = {
        0x00000b50,
        0xfffff4b0,
        0x00e700d5,
        0xfffffe21,
        0x00b90119,
        0xfffffe65,
        0x2094a539,
        0x00000000,
        0x00001000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000020,
    };
    uint32_t resCop2c[32] = {
        0x00000ffb,
        0xffb7ff44,
        0xf9ca0ebc,
        0x063700ad,
        0x00000eb7,
        0x00000000,
        0xfffffeac,
        0x00001700,
        0x00000000,
        0x00000fa0,
        0x0000f060,
        0x0000f060,
        0x00000000,
        0x00000640,
        0x00000640,
        0x00000640,
        0x0bb80fa0,
        0x0fa00fa0,
        0x0fa00bb8,
        0x0bb80fa0,
        0x00000fa0,
        0x00000000,
        0x00000000,
        0x00000000,
        0x01400000,
        0x00f00000,
        0x00000400,
        0xfffffec8,
        0x01400000,
        0x00000155,
        0x00000100,
        0x00000000,
    };
    uint32_t resCop2d[32] = {
        0x00000b50,
        0xfffff4b0,
        0x00e700d5,
        0xfffffe21,
        0x00b90119,
        0xfffffe65,
        0x2094a539,
        0x00000000,
        0x00000e08,
        0x00000bd1,
        0x000002dc,
        0x00000d12,
        0x00000000,
        0x00000000,
        0x01d003ff,
        0x01d003ff,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000d12,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00e08388,
        0x00000bd1,
        0x000002dc,
        0x00000d12,
        0x000068b7,
        0x000068b7,
        0x00000000,
        0x00000020,
    };

    void doTest() {
        command = 0x00080001;
        printf("testing GTE::RTPS...\n");
        assignRegisters(cop2c, cop2d);
        GTE.execute(command, nullptr);
        checkResult(resCop2c, resCop2d);
    }
};
class testRTPT : public testGTE {
public:
	uint32_t cop2c[32] = {
        0x00000ffb,
        0xffb7ff44,
        0xf9ca0ebc,
        0x063700ad,
        0x00000eb7,
        0x00000000,
        0xfffffeac,
        0x00001700,
        0x00000000,
        0x00000fa0,
        0x0000f060,
        0x0000f060,
        0x00000000,
        0x00000640,
        0x00000640,
        0x00000640,
        0x0bb80fa0,
        0x0fa00fa0,
        0x0fa00bb8,
        0x0bb80fa0,
        0x00000fa0,
        0x00000000,
        0x00000000,
        0x00000000,
        0x01400000,
        0x00f00000,
        0x00000400,
        0xfffffec8,
        0x01400000,
        0x00000155,
        0x00000100,
        0x00000000,
	};
	uint32_t cop2d[32] = {
        0x00e70119,
        0xfffffe65,
        0x00e700d5,
        0xfffffe21,
        0x00b90119,
        0xfffffe65,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000020,
    };
	uint32_t resCop2c[32] = {
        0x00000ffb,
        0xffb7ff44,
        0xf9ca0ebc,
        0x063700ad,
        0x00000eb7,
        0x00000000,
        0xfffffeac,
        0x00001700,
        0x00000000,
        0x00000fa0,
        0x0000f060,
        0x0000f060,
        0x00000000,
        0x00000640,
        0x00000640,
        0x00000640,
        0x0bb80fa0,
        0x0fa00fa0,
        0x0fa00bb8,
        0x0bb80fa0,
        0x00000fa0,
        0x00000000,
        0x00000000,
        0x00000000,
        0x01400000,
        0x00f00000,
        0x00000400,
        0xfffffec8,
        0x01400000,
        0x00000155,
        0x00000100,
        0x00000000,
    };
	uint32_t resCop2d[32] = {
        0x00e70119,
        0xfffffe65,
        0x00e700d5,
        0xfffffe21,
        0x00b90119,
        0xfffffe65,
        0x00000000,
        0x00000000,
        0x00001000,
        0x0000012b,
        0xfffffff0,
        0x000015d9,
        0x00f40176,
        0x00f9016b,
        0x00ed0176,
        0x00ed0176,
        0x00000000,
        0x000015eb,
        0x000015aa,
        0x000015d9,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x0106e038,
        0x0000012b,
        0xfffffff0,
        0x000015d9,
        0x00007c02,
        0x00007c02,
        0x00000000,
        0x00000020,
    };

	void doTest() {
        command = 0x00080030;
        printf("testing GTE::RTPT...\n");
        assignRegisters(cop2c, cop2d);
        GTE.execute(command, nullptr);
        checkResult(resCop2c, resCop2d);
	}
};