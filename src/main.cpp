#include <stdio.h>
#include "playstation.hpp"


#define CLOCK_SPEED (33868800 / 60)

int main(int argc, char** argv) {
    if (argc < 2) Helpers::panic("Usage: ChonkyStation [bios path]");

    printf("ChonkyStation\n");

    PlayStation playstation = PlayStation(argv[1]);

    u64 cycles = 0;
    while (true) {
        cycles = 0;
        while (cycles++ < CLOCK_SPEED)
            playstation.step();

        //printf("pc: 0x%08x\n", playstation.getPC());
        //Helpers::dump("ramdump.bin", playstation.getRAM(), 2_MB);
    }

    return 0;
}