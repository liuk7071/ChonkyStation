#include <iostream>
#include <iomanip>
#include "cpu.h"
#include "glad/glad.h"
#include "SDL.h"
#undef main
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"
int screen_width = 640, screen_height = 480;
SDL_Window* main_window = nullptr;
SDL_GLContext gl_context = nullptr;

uint32_t instr;
cpu Cpu = cpu();

int elapsed = 0;
void cycle() {
    
}

int main(int argc, char** argv) {
    
    printf("\n Executing \n \n");


    SDL_Event event;
    

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("ChonkyStation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    SDL_RenderClear(renderer);
    SDL_Texture* frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STATIC, screen_width, screen_height);

    

    bool quit = false;

    while (!quit) {
        if (elapsed >= 540672) {
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
        
        
    }
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(frame);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}


int Main()
{
    if (SDL_Init(SDL_INIT_EVENTS) < 0) {
        throw(std::string("Failed to initialize SDL: ") + SDL_GetError());
    }

    int context_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (0 == (main_window = SDL_CreateWindow("ChonkyStation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL))) {
        throw(std::string("Failed to create window: ") + SDL_GetError());
    }

    if (NULL == (gl_context = SDL_GL_CreateContext(main_window))) {
        throw(std::string("Failed to create OpenGL context"));
    }
    else SDL_GL_MakeCurrent(main_window, gl_context);


    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        throw(std::string("Failed to initialize GLAD"));
    }

    // log opengl version so it looks cool
    std::cout << std::setw(34) << std::left << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << std::endl;
    std::cout << std::setw(34) << std::left << "OpenGL Shading Language Version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << std::setw(34) << std::left << "OpenGL Vendor:" << (char*)glGetString(GL_VENDOR) << std::endl;
    std::cout << std::setw(34) << std::left << "OpenGL Renderer:" << (char*)glGetString(GL_RENDERER) << std::endl;

    // imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(main_window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
    bool loop = true;
    while (loop)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {

            ImGui_ImplSDL2_ProcessEvent(&event);


            switch (event.type)
            {
            case SDL_QUIT:
                loop = false;
                exit(0);
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {

                }
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    loop = false;
                    break;
                }
                break;
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(main_window);
        ImGui::NewFrame();
        {
            //ImGui::Begin("ChonkyStation", NULL, ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Load")) {
                        break;
                    }
                    ImGui::EndMenu();
                }


                if (ImGui::BeginMenu("Settings")) {
                    if (ImGui::BeginMenu("Video")) {
                        if (ImGui::MenuItem("Display")) {}
                        if (ImGui::MenuItem("Graphics")) {}
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem("Input")) {}
                    if (ImGui::MenuItem("Audio")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            //ImGui::End();
        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(main_window);
    }
    
    Main();
    return 0;
}