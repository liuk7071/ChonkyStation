#pragma once

#include <helpers.hpp>
#include <cpu_core.hpp>
#include <backends/interpreter/interpreter.hpp>
#include <memory.hpp>


class Cpu {
public:
    Cpu(Memory* mem) : mem(mem) {}

    Memory* mem;

    enum class Backend {
        Interpreter
    };
    Backend backend = Backend::Interpreter;

    CpuCore core;

    Interpreter interpreter;
    
    void (*stepFunc)(CpuCore*, Memory*) = &interpreter.step;
    void step();

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
};