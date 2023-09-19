#pragma once

#include <helpers.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <interrupt.hpp>
#include <dma.hpp>
#include <gpu.hpp>
#include <cdrom.hpp>
#include <scheduler.hpp>


class PlayStation {
public:
    PlayStation(const fs::path& biosPath) : cdrom(&scheduler), interrupt(), gpu(&scheduler), dma(), mem(&interrupt, &dma, &gpu, &cdrom), cpu(&mem) {
        mem.loadBios(biosPath);
        cpu.switchBackend(Cpu::Backend::Interpreter);

        // Setup GPU scheduler events
        scheduler.push(&gpu.scanlineEvent, scheduler.time + GPUConstants::cyclesPerScanline, &gpu);
    }

    u64 cycles = 0;
    // Steps the system
    void step() {
        cpu.step();
        auto cyclesToAdd = isInBIOS() ? 20 : 2;
        scheduler.tick(cyclesToAdd);
        cycles += cyclesToAdd;

        // CDROM IRQ
        if (cdrom.shouldFireIRQ()) {
            interrupt.raiseInterrupt(Interrupt::InterruptType::CDROM);
        }

        if (cpu.core.pc == 0x80030000) {
            isKernelSetupDone = true;
            if (hasToLoadExecutable) {
                loadExecutable(executablePath);
                hasToLoadExecutable = false;
            }
        }
    }

    u32 getPC() { return cpu.core.pc; }
    u8* getRAM() { return mem.ram; }
    u8* getVRAM() { return gpu.getVRAM(); }
    bool getVBLANKAndClear() {
        bool temp = gpu.vblank;
        gpu.vblank = false;
        return temp;
    }
    void VBLANK() { interrupt.raiseInterrupt(Interrupt::InterruptType::VBLANK); }
    bool isInBIOS() { return Helpers::inRangeSized<u32>(cpu.core.pc, (u32)Memory::MemoryBase::BIOS, (u32)Memory::MemorySize::BIOS); }
    
    void loadExecutable(const fs::path path) {
        auto binary = Helpers::readBinary(path);

        u32 entryPc = 0;
        u32 entryAddr = 0;
        u32 fileSize = 0;

        memcpy(&entryPc,   &binary[0x10], sizeof(u32));
        memcpy(&entryAddr, &binary[0x18], sizeof(u32));
        memcpy(&fileSize,  &binary[0x1c], sizeof(u32));

        for (int i = 0; i < (binary.size() - 2048); i++) {
            mem.write(entryAddr + i, binary[0x800 + i]);
        }

        cpu.core.pc = entryPc;
        cpu.core.nextPc = cpu.core.pc + 4;
    }

    void sideloadExecutable(const fs::path path) {
        hasToLoadExecutable = true;
        executablePath = path;
    }

private:
    Cpu cpu;
    Scheduler scheduler;
    DMA dma;
    Memory mem;
    Interrupt interrupt;
    GPU gpu;
    CDROM cdrom;

    bool hasToLoadExecutable = false;
    fs::path executablePath;

    bool isKernelSetupDone = false;
};