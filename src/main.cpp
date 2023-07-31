#include <stdio.h>

#ifndef __ANDROID__
#include <SDL.h>
#endif

#include "playstation.hpp"

#ifdef __ANDROID__
#include <jni.h>
#include "../android/app/src/main/cpp/AndroidOut.h"
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>
#include <game-activity/native_app_glue/android_native_app_glue.c>
#endif


#define CLOCK_SPEED (33868800 / 60)


#ifdef __ANDROID__
void android_main(struct android_app *pApp) {
    aout << "ChonkyStation" << std::endl;
}
#endif

int main(int argc, char** argv) {
    if (argc < 2) Helpers::panic("Usage: ChonkyStation [bios path]");

    printf("ChonkyStation\n");

    PlayStation playstation = PlayStation(argv[1]);

#ifndef __ANDROID__
    // SDL Window
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("ChonkyStation", 100, 100, 1024, 512, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);
    auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 1024, 512);
#endif

    u64 cycles = 0;
    bool running = true;
    while (running) {
        cycles = 0;
        while (cycles++ < CLOCK_SPEED)
            playstation.step();

#ifndef __ANDROID__
        // Handle SDL window events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
#endif

        u8* vram = playstation.getVRAM();
#ifndef __ANDROID__
        // Update window
        SDL_UpdateTexture(texture, nullptr, vram, 1024 * 2);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
#endif

        //printf("pc: 0x%08x\n", playstation.getPC());
        //Helpers::dump("ramdump.bin", playstation.getRAM(), 2_MB);
    }

    return 0;
}