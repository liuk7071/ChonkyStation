#pragma once

class CpuCore {
public:
    u32 pc = 0xBFC00000;
    u32 gprs[32];
    u32 hi, lo;
};