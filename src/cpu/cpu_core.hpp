#pragma once

#include <helpers.hpp>
#include <BitField.hpp>
#include <logger.hpp>
#include <interrupt.hpp>


struct COP0 {
    enum class COP0Reg {
        BPC = 0x03,
        BDA = 0x05,
        JumpDest = 0x06,
        DCIC = 0x07,
        BadVAddr = 0x08,
        BDAM = 0x09,
        BPCM = 0x0B,
        Status = 0x0C,
        Cause = 0x0D,
        EPC = 0x0E,
        PRId = 0x0F,
    };

    union {
        u32 raw;
        BitField<2, 5, u32> exc;
        BitField<8, 8, u32> ip;
        BitField<28, 2, u32> ce;
        BitField<31, 1, u32> bd;
    } cause;

    union {
        u32 raw;
        BitField<0, 1, u32> iec;
        BitField<1, 1, u32> kuc;
        BitField<2, 1, u32> iep;
        BitField<3, 1, u32> kup;
        BitField<4, 1, u32> ieo;
        BitField<5, 1, u32> kuo;
        BitField<8, 8, u32> im;
        BitField<16, 1, u32> isc;
        BitField<17, 1, u32> swcSwappedCache;
        BitField<18, 1, u32> pz;
        BitField<19, 1, u32> cm;
        BitField<20, 1, u32> pe;
        BitField<21, 1, u32> ts;
        BitField<22, 1, u32> bev;
        BitField<25, 1, u32> re;
        BitField<28, 1, u32> cu0;
        BitField<29, 1, u32> cu1;
        BitField<30, 1, u32> cu2;
        BitField<31, 1, u32> cu3;
    } status;

    u32 epc;
    u32 badVaddr;
    u32 prid = 2;

    void write(u32 cop0r, u32 data) {
        switch (cop0r) {
        case (u32)COP0Reg::BPC:         break;
        case (u32)COP0Reg::BDA:         break;
        case (u32)COP0Reg::JumpDest:    break;
        case (u32)COP0Reg::DCIC:        break;
        case (u32)COP0Reg::BDAM:        break;
        case (u32)COP0Reg::BPCM:        break;
        case (u32)COP0Reg::Status:  status.raw = data; break;
        case (u32)COP0Reg::Cause:   cause.raw = data; break;
        default:
            Helpers::panic("Unimplemented cop0 register write cop0r%d <- 0x%08x\n", cop0r, data);
        }
    }

    u32 read(u32 cop0r) {
        switch (cop0r) {
        case (u32)COP0Reg::BPC:         return 0;
        case (u32)COP0Reg::BDA:         return 0;
        case (u32)COP0Reg::JumpDest:    return 0;
        case (u32)COP0Reg::DCIC:        return 0;
        case (u32)COP0Reg::BDAM:        return 0;
        case (u32)COP0Reg::BPCM:        return 0;
        case (u32)COP0Reg::Status:      return status.raw;
        case (u32)COP0Reg::Cause:       return cause.raw;
        case (u32)COP0Reg::EPC:         return epc;
        default:
            Helpers::panic("Unimplemented cop0 register read cop0r%d\n", cop0r);
        }
    }
};

class CpuCore {
public:
	union Instruction {
	    u32 raw;
	    BitField<0, 16, u32> imm;
	    BitField<0, 26, u32> jumpImm;
        BitField<6,  5, u32> shiftImm;
	    BitField<6,  5, u32> sa;
	    BitField<11, 5, u32> rd;
	    BitField<16, 5, u32> rt;
	    BitField<21, 5, u32> rs;
	    BitField<26, 6, u32> primaryOpc;
	    BitField<0,  6, u32> secondaryOpc;
        BitField<16, 5, u32> regimmOpc;
        BitField<21, 5, u32> cop0Opc;
        BitField<0,  6, u32> func;
	};

    u32 pc = 0xbfc00000;
    u32 nextPc = pc + 4;
    u32 gprs[32];
    u32 hi, lo;
    bool branched = false;
    bool isDelaySlot = false;

    COP0 cop0;

    enum class Exception {
        INT = 0x0,
        BadFetchAddr = 0x4,
        BadStoreAddr = 0x5,
        SysCall = 0x8,
        Break = 0x9,
        Reserved_Instruction = 0xA,
        Overflow = 0xC
    };

    // If decrementPc is true, epc will be set to pc - 4 instead of pc
    void exception(Exception exception, bool decrementPc = false) {
        if (isDelaySlot)
            cop0.cause.bd = true;
        else
            cop0.cause.bd = false;

        if (exception == Exception::BadFetchAddr || exception == Exception::BadStoreAddr)
            cop0.badVaddr = pc;

        u32 handler = 0x80000080;
        if (cop0.status.bev)
            handler = 0xbfc00180;

        u32 temp = cop0.status.raw & 0x3f;
        cop0.status.raw &= ~0x3f;
        cop0.status.raw |= (temp << 2) & 0x3f;
        cop0.cause.raw &= ~0xff;
        cop0.cause.raw |= (u32)exception << 2;
        cop0.epc = pc;
        if (decrementPc)
            cop0.epc -= 4;
        if (isDelaySlot)
            cop0.epc -= 4;
        pc = handler;
        nextPc = handler + 4;
    }
    
    // Returns true if an interrupt was fired
    bool checkInterrupt(Interrupt* interrupt) {
        if (interrupt->interruptFired()) {
            cop0.cause.raw |= 1 << 10;
            if (cop0.status.iec && (cop0.status.raw & (1 << 10))) {
                exception(Exception::INT);
                return true;
            }
        }
        return false;
    }
};

namespace CpuOpcodes {
enum CpuReg {
    R0 = 0, AT = 1, V0 = 2, V1 = 3,
    A0 = 4, A1 = 5, A2 = 6, A3 = 7,
    T0 = 8, T1 = 9, T2 = 10, T3 = 11,
    T4 = 12, T5 = 13, T6 = 14, T7 = 15,
    S0 = 16, S1 = 17, S2 = 18, S3 = 19,
    S4 = 20, S5 = 21, S6 = 22, S7 = 23,
    T8 = 24, T9 = 25, K0 = 26, K1 = 27,
    GP = 28, SP = 29, S8 = 30, RA = 31,
    LO = 32, HI = 33
};

enum Opcode {
    SPECIAL = 0x00,
    REGIMM = 0x01,
    J = 0x02,
    JAL = 0x03,
    BEQ = 0x04,
    BNE = 0x05,
    BLEZ = 0x06,
    BGTZ = 0x07,
    ADDI = 0x08,
    ADDIU = 0x09,
    SLTI = 0x0A,
    SLTIU = 0x0B,
    ANDI = 0x0C,
    ORI = 0x0D,
    XORI = 0x0E,
    LUI = 0x0F,
    COP0 = 0x10,
    COP2 = 0x12,
    LB = 0x20,
    LH = 0x21,
    LWL = 0x22,
    LW = 0x23,
    LBU = 0x24,
    LHU = 0x25,
    LWR = 0x26,
    SB = 0x28,
    SH = 0x29,
    SWL = 0x2A,
    SW = 0x2B,
    SWR = 0x2E,
    LWC2 = 0x32,
    SWC2 = 0x3A
};

enum SPECIALOpcode {
    SLL = 0x00,
    SRL = 0x02,
    SRA = 0x03,
    SLLV = 0x04,
    SRLV = 0x06,
    SRAV = 0x07,
    JR = 0x08,
    JALR = 0x09,
    SYSCALL = 0x0C,
    BREAK = 0x0D,
    MFHI = 0x10,
    MTHI = 0x11,
    MFLO = 0x12,
    MTLO = 0x13,
    MULT = 0x18,
    MULTU = 0x19,
    DIV = 0x1A,
    DIVU = 0x1B,
    ADD = 0x20,
    ADDU = 0x21,
    SUB = 0x22,
    SUBU = 0x23,
    AND = 0x24,
    OR = 0x25,
    XOR = 0x26,
    NOR = 0x27,
    SLT = 0x2A,
    SLTU = 0x2B
};

enum REGIMMOpcode {
    BLTZ = 0x00,
    BGEZ = 0x01,
    BLTZAL = 0x10,
    BGEZAL = 0x11
};

enum COPOpcode {
    MF = 0x00,
    CF = 0x02,
    MT = 0x04,
    CT = 0x06,
    CO = 0x10
};

enum COP0Opcode {
    RFE = 0x10
};
}   // End namespace CpuOpcodes