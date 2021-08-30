#include <iostream>
#include <windows.h>
#include <iomanip>
#include <map>
#include <sstream>
#include "TinyFileDialogs/tinyfiledialogs.h"
#include "Cpu.h"
#include "SDL.h"
#include "glad/glad.h"
#undef main
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui_memory_editor.h"
#include "GLFW\glfw3.h"

GLFWwindow* window = nullptr;
int key, action;
bool show_settings = false;
bool show_system_settings = false;
bool show_cpu_registers = false;
bool show_ram_viewer = false;
bool started = false;
bool sideload = false;
bool run = false;
const float aspect_ratio = 640 / 480;

const char* game_path;
const char* bios_path;
const char* binary_path = "";
bool game_open, bios_selected;
bool show_dialog;
const char* dialog_message;
const char* FilterPatternsBin[1] = { "*.bin" };
const char* FilterPatternsExe[1] = { "*.exe" };

static MemoryEditor MemEditor;
static void key_callback(GLFWwindow* window, int key_, int scancode, int action_, int mods)
{
    key = key_;
    action = action_;
}

void Reset(const char* rom_dir, const char* bios_dir, cpu* Cpu) {
    if (!bios_selected) {
        dialog_message = "Please select a bios file at Settings > System";
        show_dialog = true;
    }
    else {
        if (run) {
            delete Cpu;
            Cpu = new cpu(sideload ? binary_path : "", bios_path, false);
        }
        run = true;
        started = true;
    }
}

void InitWindow() {
    const char* glsl_version = "#version 450";
    window = glfwCreateWindow(1600, 720, "ChonkyStation", nullptr, nullptr);
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
    // glBindTexture(GL_TEXTURE_2D, Cpu->bus.Gpu.id);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void SystemSettingsMenu() {
    static bool general = true;
    static bool graphics = false;
    ImGui::Begin("System Settings", &show_system_settings);
    //ImGui::NewLine();
    ImGui::Checkbox("Sideload binary", &sideload);
    
    if (!sideload) ImGui::PushDisabled();
    ImGui::Text("Binary to sideload: \"%s\"", binary_path);
    if (ImGui::Button("Select##1")) {
        if (!(binary_path = tinyfd_openFileDialog("Select a Playstation 1 binary", "", 1, FilterPatternsExe, "PS1 binary", false))) {
           binary_path = "";
        }
    }
    if (!sideload) ImGui::PopDisabled();

    ImGui::Text("BIOS path: \"%s\"", bios_path);
    if (ImGui::Button("Select##2")) {
        if (!(bios_path = tinyfd_openFileDialog("Select a Playstation 1 BIOS file", "", 1, FilterPatternsBin, "PS1 BIOS", false))) {
            bios_selected = false;
        }
        else {
            bios_selected = true;
        }
    }
    ImGui::End();
}

void Dialog() {
    ImGui::Begin("", &show_dialog);
    ImGui::Text(dialog_message);
    ImGui::End();
}

void ImGuiFrame(cpu *Cpu) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

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
    ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(Cpu->bus.Gpu.id)), image_size);
    ImGui::End();

    // MenuBar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) {
                if (!(game_path = tinyfd_openFileDialog("Select a Playstation 1 Game", "", 1, FilterPatternsBin, "PS1 Game", false))) {
                    game_open = false;
                }
                else {
                    game_open = true;
                }
            }
    
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
    
        if (ImGui::BeginMenu("Emulation")) {
            if (ImGui::MenuItem("Start", NULL, false, !started)) {
                Reset("", bios_path, Cpu);
            }
    
            if (ImGui::MenuItem(run ? "Pause" : "Resume", NULL, false, started)) {
                run = !run;
            }
    
            if (ImGui::MenuItem("Reset")) {
                run = false;
                started = false;
                Reset("", bios_path, Cpu);
            }
    
            ImGui::EndMenu();
        }
    
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("System")) {
                show_system_settings = true;
            }
            ImGui::EndMenu();
        }
    
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::BeginMenu("Memory Viewer")) {
                if (ImGui::MenuItem("RAM")) {
                    show_ram_viewer = !show_ram_viewer;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
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

    cpu* Cpu = new cpu(rom_dir, bios_dir, running_in_ci); // TODO: Have a system class, don't use the CPU as one   
    if (running_in_ci) { // When executing in CI, run the system headless, without a BIOS.
        Cpu->sideloadExecutable(rom_dir);
        while (true) // The CI tests write to a custom register that force-closes the emulator when they're done, so this will run until that is hit
            Cpu->step();
    }

    InitWindow();
    
    // OpenGL
    //glGenTextures(1, &Cpu->bus.Gpu.id);
    //glBindTexture(GL_TEXTURE_2D, Cpu->bus.Gpu.id);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    Cpu->bus.Gpu.InitGL();

    double prevTime = glfwGetTime();
    int frameCount = 0;
    while (!glfwWindowShouldClose(window)) {
        if (Cpu->frame_cycles >= 500000 || !run) {
            glfwPollEvents();

            if (keyboard[SDLK_RETURN]) {   // Manually fire a VBLANK IRQ to test some programs (I know)
                Cpu->bus.mem.I_STAT &= ~1;
                Cpu->bus.mem.I_STAT |= 1;
            }

            //Update(Cpu);
            ImGuiFrame(Cpu);
            if (show_system_settings) SystemSettingsMenu();
            if (show_dialog) Dialog();
            if (show_ram_viewer) MemEditor.DrawWindow("RAM Viewer", Cpu->bus.mem.ram, 0x200000);

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            if(run) Cpu->frame_cycles = 0;

            frameCount++;
            double currTime = glfwGetTime();
            if (currTime - prevTime >= 1.0) {
                std::stringstream title;
                title << "ChonkyStation " << "[" << frameCount << " FPS]";
                glfwSetWindowTitle(window, title.str().c_str());
                frameCount = 0;
                prevTime = currTime;
            }
        }
        else {
            Cpu->step();
        }   
    }

    return 0;
}