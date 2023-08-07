#include <stdio.h>
#include <SDL.h>
#include "playstation.hpp"


constexpr auto cyclesPerFrame = 33868800 / 60;

int main(int argc, char** argv) {
    if (argc < 2) Helpers::panic("Usage: ChonkyStation [bios path]\n");

    printf("ChonkyStation\n");

    PlayStation playstation = PlayStation(argv[1]);

    // SDL Window
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("ChonkyStation", 100, 100, 1024, 512, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);
    auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 1024, 512);

    u64 cycles = 0;
    bool running = true;
    while (running) {
        cycles = 0;
 
        while (cycles < cyclesPerFrame) {
            playstation.step();
            if (!playstation.isInBIOS())
                cycles += 2;
            else
                cycles += 20;
        }

        // VBLANK interrupt
        playstation.VBLANK();

        // Handle SDL window events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        u8* vram = playstation.getVRAM();
        // Update window
        SDL_UpdateTexture(texture, nullptr, vram, 1024 * 2);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        //printf("pc: 0x%08x\n", playstation.getPC());
        //Helpers::dump("ramdump.bin", playstation.getRAM(), 2_MB);
    }

    return 0;
}