#include <iostream>
#include <windows.h>
#include <iomanip>
#include <map>
#include "cpu.h"
#include "SDL.h"
#include "glad/glad.h"
#undef main
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"

int main(int argc, char** argv) {
    std::map<int, bool> keyboard;
    keyboard[SDLK_RETURN] = true;

    printf("\n Executing \n \n");
    // Parse CLI args (TODO: Use a library)
    const auto rom_dir = argc > 1 ? std::string(argv[1]) : "";  // Path of the ROM (Or "" if we just want to run the BIOS)
    const bool running_in_ci = argc > 2 && std::string(argv[2]).compare("--continuous-integration") == 0; // Running in CI makes the system run without SDL 
    const std::string bios_dir = running_in_ci ? "" : "./SCPH1001.bin"; // In CI, don't load a BIOS, otherwise load SCPH1001.bin. TODO: Add a CLI arg for the BIOS path

    auto Cpu = cpu (rom_dir, bios_dir, running_in_ci); // TODO: Have a system class, don't use the CPU as one   
    if (running_in_ci) { // When executing in CI, run the system headless, without a BIOS.
        Cpu.sideloadExecutable (rom_dir);
        while (true) // The CI tests write to a custom register that force-closes the emulator when they're done, so this will run until that is hit
            Cpu.step();
    }


    SDL_Event event;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
    const auto window = SDL_CreateWindow("ChonkyStation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    const auto renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderClear(renderer);
    const auto frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 480);
  
    bool quit = false;
    while (!quit) {
        if (Cpu.frame_cycles >= 500000) {
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    keyboard[event.key.keysym.sym] = false;
                    break;
                case SDL_KEYUP:
                    keyboard[event.key.keysym.sym] = true;
                    break;
                }
            }
            if (!keyboard[SDLK_RETURN]) {   // Manually fire a VBLANK IRQ to test some programs (I know)
                Cpu.bus.mem.I_STAT &= ~1;
                Cpu.bus.mem.I_STAT |= 1;
            }

            SDL_UpdateTexture(frame, NULL, Cpu.bus.Gpu.pixels, 640 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, frame, NULL, NULL);
            SDL_RenderPresent(renderer);
            Cpu.frame_cycles = 0;
        }

        Cpu.step();
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


