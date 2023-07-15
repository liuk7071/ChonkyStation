#pragma once

#include <helpers.hpp>
#include <BitField.hpp>
#include <logger.hpp>


class CpuCore {
public:
	union Instruction {
	    u32 raw;
	    BitField<0, 16, u32> imm;
	    BitField<0, 26, u32> jumpImm;
	    BitField<6,  5, u32> sa;
	    BitField<11, 5, u32> rd;
	    BitField<16, 5, u32> rt;
	    BitField<21, 5, u32> rs;
	    BitField<26, 6, u32> primaryOpc;
	    BitField<0,  6, u32> secondaryOpc;
	};

    u32 pc = 0xbfc00000;
    u32 gprs[32];
    u32 hi, lo;

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
        SWC2 = 0x3A,
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
        SLTU = 0x2B,
    };

    enum REGIMMOpcode {
        BLTZ = 0x00,
        BGEZ = 0x01,
        BLTZAL = 0x10,
        BGEZAL = 0x11,
    };

    enum COPOpcode {
        MF = 0x00,
        CF = 0x02,
        MT = 0x04,
        CT = 0x06,
        CO = 0x10,
    };

    enum COP0Opcode {
        RFE = 0x10,
    };

	// Disassembler
    MAKE_LOG_FUNCTION(log, cpuTraceLogger)

	std::string gprNames[32] = { "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
	                        "$t0", "$t1", "$t2", "$t3","$t4", "$t5", "$t6", "$t7",
	                        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	                        "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };

	void disassemble(Instruction instr) {
        switch (instr.primaryOpc) {
        case Opcode::LUI: log("0x%08x: lui %s, 0x%04x\n", pc, gprNames[instr.rt].c_str(), instr.imm.Value()); break;
        default: log("0x%08x: (not disassembled primary opc 0x%02x)\n", pc, instr.primaryOpc.Value());
        }
	}
};