#include "disassembler.hpp"


void Disassembler::disassemble(CpuCore::Instruction instr, CpuCore* core) {
    switch ((CpuCore::Opcode)instr.primaryOpc.Value()) {
    case CpuCore::Opcode::SPECIAL: {
        switch ((CpuCore::SPECIALOpcode)instr.secondaryOpc.Value()) {
        case CpuCore::SPECIALOpcode::SLL:    log("0x%08x: sll    %s, %s, 0x%04x\n", core->pc, gprNames[instr.rd].c_str(), gprNames[instr.rt].c_str(), instr.shiftImm.Value()); break;
        case CpuCore::SPECIALOpcode::ADD:    log("0x%08x: add    %s, %s, %s\n", core->pc, gprNames[instr.rd].c_str(), gprNames[instr.rs].c_str(), gprNames[instr.rt].c_str()); break;
        case CpuCore::SPECIALOpcode::JR:     log("0x%08x: jr     %s\n", core->pc, gprNames[instr.rs].c_str()); break;
        case CpuCore::SPECIALOpcode::JALR:   log("0x%08x: jalr   %s\n", core->pc, gprNames[instr.rs].c_str()); break;
        case CpuCore::SPECIALOpcode::ADDU:   log("0x%08x: addu   %s, %s, %s\n", core->pc, gprNames[instr.rd].c_str(), gprNames[instr.rs].c_str(), gprNames[instr.rt].c_str()); break;
        case CpuCore::SPECIALOpcode::SLTU:   log("0x%08x: sltu   %s, %s, %s\n", core->pc, gprNames[instr.rd].c_str(), gprNames[instr.rs].c_str(), gprNames[instr.rt].c_str()); break;
        default:                    log("0x%08x: (not disassembled secondary opc 0x%02x)\n", core->pc, instr.secondaryOpc.Value());
        }
        break;
    }
    case CpuCore::Opcode::J:                 log("0x%08x: j      0x%08x\n", core->pc, instr.jumpImm.Value()); break;
    case CpuCore::Opcode::JAL:               log("0x%08x: jal    0x%08x\n", core->pc, instr.jumpImm.Value()); break;
    case CpuCore::Opcode::BNE:               log("0x%08x: bne    %s, %s, 0x%04x\n", core->pc, gprNames[instr.rs].c_str(), gprNames[instr.rt].c_str(), instr.imm.Value()); break;
    case CpuCore::Opcode::ADDI:              log("0x%08x: addi   %s, %s, 0x%04x\n", core->pc, gprNames[instr.rt].c_str(), gprNames[instr.rs].c_str(), instr.imm.Value()); break;
    case CpuCore::Opcode::ADDIU:             log("0x%08x: addiu  %s, %s, 0x%04x\n", core->pc, gprNames[instr.rt].c_str(), gprNames[instr.rs].c_str(), instr.imm.Value()); break;
    case CpuCore::Opcode::ANDI:              log("0x%08x: andi   %s, %s, 0x%04x\n", core->pc, gprNames[instr.rt].c_str(), gprNames[instr.rs].c_str(), instr.imm.Value()); break;
    case CpuCore::Opcode::ORI:               log("0x%08x: ori    %s, %s, 0x%04x\n", core->pc, gprNames[instr.rt].c_str(), gprNames[instr.rs].c_str(), instr.imm.Value()); break;
    case CpuCore::Opcode::LUI:               log("0x%08x: lui    %s, 0x%04x\n", core->pc, gprNames[instr.rt].c_str(), instr.imm.Value()); break;
    case CpuCore::Opcode::SB:                log("0x%08x: sb     %s, 0x%04x(%s)      ; addr: 0x%08x\n", core->pc, gprNames[instr.rt].c_str(), instr.imm.Value(), gprNames[instr.rs].c_str(), core->gprs[instr.rs] + (u32)(s16)instr.imm); break;
    case CpuCore::Opcode::SH:                log("0x%08x: sh     %s, 0x%04x(%s)      ; addr: 0x%08x\n", core->pc, gprNames[instr.rt].c_str(), instr.imm.Value(), gprNames[instr.rs].c_str(), core->gprs[instr.rs] + (u32)(s16)instr.imm); break;
    case CpuCore::Opcode::SW:                log("0x%08x: sw     %s, 0x%04x(%s)      ; addr: 0x%08x\n", core->pc, gprNames[instr.rt].c_str(), instr.imm.Value(), gprNames[instr.rs].c_str(), core->gprs[instr.rs] + (u32)(s16)instr.imm); break;
    default:                        log("0x%08x: (not disassembled primary opc 0x%02x)\n", core->pc, instr.primaryOpc.Value());
    }
}