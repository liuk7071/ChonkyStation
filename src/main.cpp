#include <stdio.h>

//#ifndef __ANDROID__
#include <SDL.h>
//#endif

#include "playstation.hpp"

#ifdef __ANDROID__
#include <unistd.h>
#include <jni.h>
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>
#include <game-activity/native_app_glue/android_native_app_glue.c>
#include <android/asset_manager.h>
#include "../android/app/src/main/cpp/chonk_renderer.h"

Renderer* renderer = new Renderer();
u8* biosBuf = new u8[512_KB];
bool biosPicked = false;
bool startEmulator = false;

extern "C" JNIEXPORT void JNICALL Java_com_example_chonkystation_MainActivity_00024Companion_startEmulator(JNIEnv* env, jclass) {
    startEmulator = true;
}
// TODO: Move all of this to another file
extern "C" JNIEXPORT jstring JNICALL Java_com_example_chonkystation_MainActivity_00024Companion_readFile(
    JNIEnv* env, jclass, jint fd) {
    aout << "If you see this a file was picked and I am reading it" << std::endl;
    if (fd < 0) {
        return env->NewStringUTF("Invalid fd");
    }

    std::memset(biosBuf, 0, 512_KB);
    FILE* file = fdopen(dup(fd), "rb");
    if(file == NULL) {
        Helpers::panic("error opening file\n");
    }
    rewind(file);
    fseek(file, 0, SEEK_SET);
    fread(biosBuf, 1, 512_KB, file);
    for(int i = 0; i < 10; i++)
        aout << std::hex << biosBuf[i] << std::endl;

    const auto err = ferror(file);
    if(err)
        Helpers::panic("[FATAL] ferror returned %d\n", err);
    biosPicked = true;
    return env->NewStringUTF("Dummy string");
}

PlayStation* playstation = new PlayStation("");

void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            aout << "APP_CMD_INIT_WINDOW !!!!!!!" << std::endl;
            pApp->userData = playstation;
            renderer->init(pApp);
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being destroyed. Use this to clean up your userData to avoid leaking
            // resources.
            //
            // We have to check if userData is assigned just in case this comes in really quickly
            if (pApp->userData) {
                //
                //auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                //pApp->userData = nullptr;
                //delete pRenderer;
            }
            break;
        default:
            break;
    }
}

#endif

#define CLOCK_SPEED (33868800 / 60)

int main(int argc, char** argv) {
    #ifndef __ANDROID__
    if (argc < 2) Helpers::panic("Usage: ChonkyStation [bios path]");
    #endif

    printf("ChonkyStation\n");

    PlayStation playstation = PlayStation(argv[1]);

#ifndef __ANDROID__
    // SDL Window
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("ChonkyStation", 100, 100, 1024, 512, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);
    auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_STREAMING, 1024, 512);
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

#ifdef __ANDROID__
u64 cycles = 0;
bool canExecute = false;
void android_main(struct android_app *pApp) {
    aout << "ChonkyStation" << std::endl;
    renderer->vramData = playstation->getVRAM();
    // SDL Window
    /*if(SDL_Init(SDL_INIT_VIDEO)) {
        aout << SDL_GetError() << std::endl;
        Helpers::panic("[FATAL] SDL_Init failed\n");
    }
    SDL_Window* window = SDL_CreateWindow("ChonkyStation", 100, 100, 1024, 512, 0);
    auto renderer = SDL_CreateRenderer(window, -1, 0);
    auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 1024, 512);*/

    pApp->onAppCmd = handle_cmd;
    int events;
    android_poll_source *pSource;
    do {
        // Process all pending events before running game logic.
        if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
            if (pSource) {
                pSource->process(pApp, pSource);
            }
        }

        // Check if any user data is associated. This is assigned in handle_cmd
        if (pApp->userData) {

            // We know that our user data is a Renderer, so reinterpret cast it. If you change your
            // user data remember to change it here
            auto* playstation = reinterpret_cast<PlayStation*>(pApp->userData);
            // Check if a bios file was selected
            if(biosPicked) {
                //renderer->init(pApp);
                biosPicked = false;
                canExecute = true;
                std::memcpy(playstation->getBIOS(), biosBuf, 512_KB);
                aout << "BIOS loaded." << std::endl;
            }
            if(canExecute && startEmulator) {
                cycles = 0;
                while(cycles++ < CLOCK_SPEED) {
                    playstation->step();
                }

                u8* vram = playstation->getVRAM();
                renderer->vramData = vram;
                // TODO: Display VRAM here somehow ??????

                aout << "Calling render ..." << std::endl;
                renderer->render();
                aout << "Returned from render" << std::endl;
                const auto error = glGetError();
                if(error)
                    Helpers::panic("[FATAL] GL Error: %d\n", error);
            }

            // Process game input
            //pRenderer->handleInput();

            // Render a frame
            //pRenderer->render();
        }
    } while (!pApp->destroyRequested);
    //main(1, 0);
}
#endif