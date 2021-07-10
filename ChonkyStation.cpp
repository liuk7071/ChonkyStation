#include <iostream>
#include <windows.h>
#include <iomanip>
#include <map>
#include "cpu.h"
#include "SDL.h"
#include "glad/glad.h"
#undef main
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "GLFW\glfw3.h"

GLFWwindow* window = nullptr;
unsigned int id;
int key, action;
bool show_demo_window = true;
const float aspect_ratio = 1024 / 512;
static void key_callback(GLFWwindow* window, int key_, int scancode, int action_, int mods)
{
    key = key_;
    action = action_;
}

void InitWindow() {
    const char* glsl_version = "#version 450";
    window = glfwCreateWindow(640, 480, "ChonkyStation", nullptr, nullptr);
    glfwSetWindowPos(window, 100, 100);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGL();

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}
void Update(cpu* Cpu) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, Cpu->bus.Gpu.rast.vram_rgb);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void ImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Demo
    if(show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // Display
    bool show = false;
    ImGui::Begin("Image", &show, ImGuiWindowFlags_NoTitleBar);

    float x = ImGui::GetWindowSize().x - 15, y = ImGui::GetWindowSize().y - 15;
    float current_aspect_ratio = x / y;
    if (aspect_ratio > current_aspect_ratio) {
        y = x / aspect_ratio;
    }
    else {
        x = y * aspect_ratio;
    }

    ImVec2 image_size(x, y);
    ImVec2 centered((ImGui::GetWindowSize().x - image_size.x) * 0.5, (ImGui::GetWindowSize().y - image_size.y) * 0.5);
    ImGui::SetCursorPos(centered);
    ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(id)), image_size);
    ImGui::End();
}

int main(int argc, char** argv) {
    if (!glfwInit())
    {
        exit(1);
    }
    std::map<int, bool> keyboard;
    printf("\n Executing \n \n");
    // Parse CLI args (TODO: Use a library)
    const auto rom_dir = argc > 1 ? std::string(argv[1]) : "";  // Path of the ROM (Or "" if we just want to run the BIOS)
    const bool running_in_ci = argc > 2 && std::string(argv[2]).compare("--continuous-integration") == 0; // Running in CI makes the system run without SDL 
    const std::string bios_dir = running_in_ci ? "" : "./SCPH1001.bin"; // In CI, don't load a BIOS, otherwise load SCPH1001.bin. TODO: Add a CLI arg for the BIOS path

    auto Cpu = cpu(rom_dir, bios_dir, running_in_ci); // TODO: Have a system class, don't use the CPU as one   
    if (running_in_ci) { // When executing in CI, run the system headless, without a BIOS.
        Cpu.sideloadExecutable(rom_dir);
        while (true) // The CI tests write to a custom register that force-closes the emulator when they're done, so this will run until that is hit
            Cpu.step();
    }

    InitWindow();
    // OpenGL
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, Cpu.bus.Gpu.rast.vram_rgb);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    while (!glfwWindowShouldClose(window)) {
        if (Cpu.frame_cycles >= 500000) {
            glfwPollEvents();

            if (keyboard[SDLK_RETURN]) {   // Manually fire a VBLANK IRQ to test some programs (I know)
                Cpu.bus.mem.I_STAT &= ~1;
                Cpu.bus.mem.I_STAT |= 1;
            }

            Update(&Cpu);
            ImGuiFrame();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            Cpu.frame_cycles = 0;
        }

        Cpu.step();
    }

    return 0;
}



