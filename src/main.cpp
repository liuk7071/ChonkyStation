#include <stdio.h>

#include "playstation.hpp"


int main(int argc, char** argv) {
    if (argc < 2) Helpers::panic("Usage: ChonkyStation [bios path]");

    printf("ChonkyStation\n");

    PlayStation playstation = PlayStation(argv[1]);

    while(true)
        playstation.step();

    return 0;
}