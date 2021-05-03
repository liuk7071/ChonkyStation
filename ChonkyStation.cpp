#include <iostream>
#include <iomanip>
#include "cpu.h"
#include "SDL.h"
#include "glad/glad.h"
#undef main
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"

int main(int argc, char** argv) {
    printf("\n Executing \n \n");
    const auto rom_dir = argc > 1 ? std::string(argv[1]) : ""; 
    auto Cpu = cpu(rom_dir); // TODO: Have a system class, don't use the CPU as oen

    SDL_Event event;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
    const auto window = SDL_CreateWindow("ChonkyStation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    const auto renderer = SDL_CreateRenderer(window, -1, 0);
    
    SDL_RenderClear(renderer);
    const auto frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 480);

   // SDL_Window* window_vram = SDL_CreateWindow("VRAM Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2048, 512, SDL_WINDOW_SHOWN);
   // SDL_Renderer* renderer_vram = SDL_CreateRenderer(window_vram, -1, 0);
   //
   // SDL_RenderClear(renderer_vram);
   // SDL_Texture* frame_vram = SDL_CreateTexture(renderer_vram, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STATIC, 2048, 512);
  
  
    uint32_t instr = 0; // TODO: Move to CPU class
    int elapsed = 0;    // TODO: Move to CPU class
    bool quit = false;

    while (!quit) {
        if (elapsed >= 500000) {
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                }
            }
            SDL_UpdateTexture(frame, NULL, Cpu.bus.Gpu.pixels, 640 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, frame, NULL, NULL);
            SDL_RenderPresent(renderer);
            elapsed = 0;
        }

        instr = Cpu.fetch(Cpu.pc);
        if (Cpu.debug) printf("0x%.8X | 0x%.8X: ", Cpu.pc, instr);
        Cpu.execute(instr);
        elapsed++;
        Cpu.check_dma();
        
        
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


