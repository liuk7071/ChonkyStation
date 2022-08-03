#include <iostream>
#define NOMINMAX
#include <windows.h>
#include <sstream>
#include <map>
#include "TinyFileDialogs/tinyfiledialogs.h"
#include "glad/glad.h"
#undef main
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui_memory_editor.h"
#include "logwindow.h"
#include "Shader Editor/shader-editor.h"
#include "GLFW\glfw3.h"
#include "xbyak/xbyak.h"
#include "Cpu.h"

#include "scheduler.h"

GLFWwindow* window = nullptr;
bool show_settings = false;
bool show_system_settings = false;
bool show_pad_settings = false;
bool show_cpu_registers = false;
bool show_interrupt_debugger = false;
bool show_ram_viewer = false;
bool show_vram = false;
bool show_log = false;
bool show_shader_editor = false;
bool show_textured_primitives_shader_editor = false;
bool show_timer0_debugger = false;
bool show_timer1_debugger = false;
bool show_timer2_debugger = false;
bool started = false;
bool sideload = false;
bool run = false;
const float aspect_ratio = 640 / 480;
const float vram_aspect_ratio = 1024 / 512;
bool mouse_captured = false;

char* game_path;
static const std::map<uint32_t, std::string> knownBioses = {
        {0x1002e6b5, "SCPH-1002 (EU)"},
        {0x1ac46cf1, "SCPH-5000 (JP)"},
        {0x24e21a0e, "SCPH-7003 (US)"},
        {0x38f5c1fe, "SCPH-1000 (JP)"},
        {0x42ea6879, "SCPH-1002 - DTLH-3002 (EU)"},
        {0x48ba1524, "????"},
        {0x4e501b56, "SCPH-5502 - SCPH-5552 (2) (EU)"},
        {0x560e2da1, "????"},
        {0x61e5b760, "SCPH-7001 (US)"},
        {0x649db764, "SCPH-7502 (EU)"},
        {0x68d2dd36, "????"},
        {0x68ee15cc, "SCPH-5502 - SCPH-5552 (EU)"},
        {0x7de956a4, "SCPH-101"},
        {0x80a156a8, "????"},
        {0x9e7d4faa, "SCPH-3000 (JP)"},
        {0x9eff111b, "SCPH-7000 (JP)"},
        {0xa6cf18fe, "SCPH-5500 (JP)"},
        {0xa8e56981, "SCPH-3500 (JP)"},
        {0xb6ef0d64, "????"},
        {0xd10b6509, "????"},
        {0xdaa2e0a6, "SCPH-1001 - DTLH-3000 (US)"},
        {0xe7ca4fad, "????"},
        {0xf380c9ff, "SCPH-5000"},
        {0xfb4afc11, "SCPH-5000 (2)"},
};
std::string bios_path = "";
std::string binary_path = "";
bool game_open, bios_selected;
bool show_dialog;
const char* dialog_message;
const char* FilterPatternsBin[1] = { "*.bin" };
const char* FilterPatternsExe[1] = { "*.exe" };

unsigned int vram_viewer;
bool test = false;
bool tmr1irq = false;
bool spuirq = false;
bool padirq = false;

static MemoryEditor MemEditor;

bool pad1_connected = true;
bool pad2_connected = false;
std::string pad1_type = "Digital";
std::string pad2_type = "Digital";
std::string pad1_source = "Keyboard";
std::string pad2_source = "Keyboard";
uint16_t P1buttons = 0xffff;
uint16_t P2buttons = 0xffff;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Capture mouse / release mouse
    if ((mods & GLFW_MOD_SHIFT) && (mods & GLFW_MOD_CONTROL) && (mods & GLFW_MOD_ALT)) {
        if (mouse_captured) {
            // Disable mouse capture
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            // Enable mouse capture
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        mouse_captured = !mouse_captured;
    }

    int count;
    const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
    
    // Timer 1 stub
    if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        tmr1irq = true;
    }
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        spuirq = !spuirq;
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        padirq = true;
    }

    // pad1
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        P1buttons &= ~(0b0100000000000000);
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        P1buttons |= 0b0100000000000000;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        P1buttons &= ~(0b1000000000000000);
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        P1buttons |= 0b1000000000000000;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        P1buttons &= ~(0b0010000000000000);
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        P1buttons |= 0b0010000000000000;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        P1buttons &= ~(0b0001000000000000);
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        P1buttons |= 0b0001000000000000;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        P1buttons &= ~(0b0000000000010000);
    }
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
        P1buttons |= 0b0000000000010000;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        P1buttons &= ~(0b0000000000100000);
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
        P1buttons |= 0b0000000000100000;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        P1buttons &= ~(0b0000000001000000);
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
        P1buttons |= 0b0000000001000000;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        P1buttons &= ~(0b0000000010000000);
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
        P1buttons |= 0b0000000010000000;
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        P1buttons &= ~(0b0000000000000001);
    }
    if (key == GLFW_KEY_N && action == GLFW_RELEASE) {
        P1buttons |= 0b0000000000000001;
    }

    // pad2
    if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS) {
        P2buttons &= ~(0b0000000000010000);
    }
    if (key == GLFW_KEY_KP_5 && action == GLFW_RELEASE) {
        P2buttons |= 0b0000000000010000;
    }
    if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS) {
        P2buttons &= ~(0b0000000000100000);
    }
    if (key == GLFW_KEY_KP_3 && action == GLFW_RELEASE) {
        P2buttons |= 0b0000000000100000;
    }
    if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS) {
        P2buttons &= ~(0b0000000001000000);
    }
    if (key == GLFW_KEY_KP_2 && action == GLFW_RELEASE) {
        P2buttons |= 0b0000000001000000;
    }
    if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS) {
        P2buttons &= ~(0b0000000010000000);
    }
    if (key == GLFW_KEY_KP_1 && action == GLFW_RELEASE) {
        P2buttons |= 0b0000000010000000;
    }
}

void joystick_callback(int jid, int event) {
    if (event == GLFW_CONNECTED) {
        printf("Gamepad connected\n");
    }
    else if (event == GLFW_DISCONNECTED) {
        printf("Gamepad disconnected");
    }
    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
        printf("rpog");
    }
}

void ScheduleVBLANK(void* dataptr) {
    memory* memoryptr = (memory*)dataptr;
    memoryptr->I_STAT |= 1;
    memoryptr->CDROM.Scheduler.push(&ScheduleVBLANK, memoryptr->CDROM.Scheduler.time + (33868800 / 60), dataptr);
}
void Reset(const char* rom_dir, const char* bios_dir, cpu *Cpu) {
    if (!bios_selected) {
        dialog_message = "Please select a bios file at Settings > System";
        show_dialog = true;
    }
    else {
        //Cpu->bus.mem.CDROM.Scheduler.push(&ScheduleVBLANK, Cpu->bus.mem.CDROM.Scheduler.time + (33868800 / 60), &Cpu->bus.mem);
        if (run) {
            *Cpu = cpu(sideload ? binary_path : "", bios_path, false);
            Cpu->bus.Gpu.InitGL();
            if (game_path) Cpu->bus.mem.CDROM.cd.OpenFile(game_path);
        }
        else {
            if (sideload) {
                Cpu->exe = true;
                Cpu->rom_directory = binary_path;
            }
            if(game_path) Cpu->bus.mem.CDROM.cd.OpenFile(game_path);
            run = true;
            started = true;
        }
    }
}

void InitWindow() {
    const char* glsl_version = "#version 450";
    window = glfwCreateWindow(1600, 720, "ChonkyStation", nullptr, nullptr);
    glfwSetWindowPos(window, 100, 100);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetJoystickCallback(joystick_callback);
    gladLoadGL();

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void SystemSettingsMenu(cpu* Cpu) {
    static bool general = true;
    static bool graphics = false;
    ImGui::Begin("System Settings", &show_system_settings);
    ImGui::Text("Note: Changing these settings might require a \nreset of the emulator (Emulation > Reset)");
    ImGui::NewLine();
    ImGui::Checkbox("Sideload binary", &sideload);
    
    if (!sideload) ImGui::BeginDisabled();
    ImGui::Text("Binary to sideload: \"%s\"", binary_path.c_str());
    if (ImGui::Button("Select##1")) {
        const char* path;
        if (!(path = tinyfd_openFileDialog("Select a Playstation 1 binary", "", 1, FilterPatternsExe, "PS1 binary", false))) {
            binary_path = "";
        }
        else {
            binary_path = path;
        }
    }
    if (!sideload) ImGui::EndDisabled();

    ImGui::Text("BIOS path: \"%s\"", bios_path.c_str());
    if (ImGui::Button("Select##2")) {
        const char* path;
        if (!(path = tinyfd_openFileDialog("Select a Playstation 1 BIOS file", "", 1, FilterPatternsBin, "PS1 BIOS", false))) {
            bios_selected = false;
        }
        else {
            bios_path = path;
            Cpu->bus.mem.loadBios(bios_path);
            bios_selected = true;
        }
    }

    auto type = knownBioses.find(Cpu->bus.mem.adler32bios);
    if (bios_selected) {
        if ((type != knownBioses.end())) {
            ImGui::Text("Detected %s BIOS", type->second.c_str());
        }
        else if (strncmp((const char*)&Cpu->bus.mem.bios.data()[0x78], "OpenBIOS", 8) == 0) {
            ImGui::Text("Detected OpenBIOS");
        }
        else {
            ImGui::Text("Warning: an unknown bios was selected. Things may break.");
        }
    }
    ImGui::End();
}

void PadSettingsMenu() {
    ImGui::Begin("Configure Pads", &show_pad_settings, ImGuiWindowFlags_NoResize);
    if (ImGui::BeginTabBar("pads")) {
        if (ImGui::BeginTabItem("Pad 1")) {
            ImGui::Checkbox("Connected", &pad1_connected);
            if (ImGui::BeginCombo("Type", pad1_type.c_str())) {
                if (ImGui::Selectable("Digital", "Digital" == pad1_type.c_str())) {
                    pad1_type = "Digital";
                }
                if (ImGui::Selectable("Mouse", "Mouse" == pad1_type.c_str())) {
                    pad1_type = "Mouse";
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();
            if (ImGui::BeginCombo("Input source", pad1_source.c_str())) {
                if (ImGui::Selectable("Keyboard", "Keyboard" == pad1_source.c_str())) {
                    pad1_source = "Keyboard";
                }
                if (ImGui::Selectable("Gamepad", "Gamepad" == pad1_source.c_str())) {
                    pad1_source = "Gamepad";
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Pad 2")) {
            ImGui::Checkbox("Connected", &pad2_connected);
            if (ImGui::BeginCombo("Type", pad2_type.c_str())) {
                if (ImGui::Selectable("Digital", "Digital" == pad2_type.c_str())) {
                    pad2_type = "Digital";
                }
                if (ImGui::Selectable("Mouse", "Mouse" == pad2_type.c_str())) {
                    pad2_type = "Mouse";
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();
            if (ImGui::BeginCombo("Input source", pad2_source.c_str())) {
                if (ImGui::Selectable("Keyboard", "Keyboard" == pad2_source.c_str())) {
                    pad2_source = "Keyboard";
                }
                if (ImGui::Selectable("Gamepad", "Gamepad" == pad2_source.c_str())) {
                    pad2_source = "Gamepad";
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void CpuDebugger(cpu *Cpu) {
    ImGui::Begin("CPU Registers");
    ImGui::Text("pc: 0x%08x", Cpu->pc);

    ImGui::Text("%s: 0x%08x", Cpu->reg[0], Cpu->regs[0]);
    ImGui::SameLine();
    ImGui::Text("     %s: 0x%08x", Cpu->reg[1], Cpu->regs[1]);
    for (int i = 2; i < 32; i++) {
        ImGui::Text("%s: 0x%08x", Cpu->reg[i], Cpu->regs[i]);
        ImGui::SameLine();
        i++;
        ImGui::Text("       %s: 0x%08x", Cpu->reg[i], Cpu->regs[i]);
    }
    ImGui::End();
}

void InterruptDebugger(cpu* Cpu) {
    bool VBLANK = Cpu->bus.mem.I_MASK & 0b1;
    bool GPU = Cpu->bus.mem.I_MASK & 0b10;
    bool CDROM = Cpu->bus.mem.I_MASK & 0b100;
    bool DMA = Cpu->bus.mem.I_MASK & 0b1000;
    bool TMR0 = Cpu->bus.mem.I_MASK & 0b10000;
    bool TMR1 = Cpu->bus.mem.I_MASK & 0b100000;
    bool TMR2 = Cpu->bus.mem.I_MASK & 0b1000000;
    bool PAD = Cpu->bus.mem.I_MASK & 0b10000000;
    bool SIO = Cpu->bus.mem.I_MASK & 0b100000000;
    bool SPU = Cpu->bus.mem.I_MASK & 0b1000000000;

    ImGui::Begin("Enabled Interrupts");
    ImGui::Text("VBLANK");
    ImGui::SameLine();
    ImGui::Checkbox("", &VBLANK);
    ImGui::Text("GPU   ");
    ImGui::SameLine();
    ImGui::Checkbox("", &GPU);
    ImGui::Text("CDROM ");
    ImGui::SameLine();
    ImGui::Checkbox("", &CDROM);
    ImGui::Text("DMA   ");
    ImGui::SameLine();
    ImGui::Checkbox("", &DMA);
    ImGui::Text("TMR0  ");
    ImGui::SameLine();
    ImGui::Checkbox("", &TMR0);
    ImGui::Text("TMR1  ");
    ImGui::SameLine();
    ImGui::Checkbox("", &TMR1);
    ImGui::Text("TMR2  ");
    ImGui::SameLine();
    ImGui::Checkbox("", &TMR2);
    ImGui::Text("PAD   ");
    ImGui::SameLine();
    ImGui::Checkbox("", &PAD);
    ImGui::Text("SIO   ");
    ImGui::SameLine();
    ImGui::Checkbox("", &SIO);
    ImGui::Text("SPU   ");
    ImGui::SameLine();
    ImGui::Checkbox("", &SPU);
    ImGui::End();
}

template<int timer>
void TimerDebugger(cpu* Cpu) {
    uint16_t current_value = 0;
    uint16_t counter_mode = 0;
    uint16_t target_value = 0;
    
    std::string title;
    if constexpr (timer == 0) {
        current_value = Cpu->bus.mem.tmr0.current_value;
        counter_mode = Cpu->bus.mem.tmr0.counter_mode;
        target_value = Cpu->bus.mem.tmr0.target_value;
        title = "Timer 0 Debugger";
    }
    else if constexpr (timer == 1) {
        current_value = Cpu->bus.mem.tmr1.current_value;
        counter_mode = Cpu->bus.mem.tmr1.counter_mode;
        target_value = Cpu->bus.mem.tmr1.target_value;
        title = "Timer 1 Debugger";
    }
    else if constexpr (timer == 2) {
        current_value = Cpu->bus.mem.tmr2.current_value;
        counter_mode = Cpu->bus.mem.tmr2.counter_mode;
        target_value = Cpu->bus.mem.tmr2.target_value;
        title = "Timer 2 Debugger";
    }

    ImGui::Begin(title.c_str());
    ImGui::Text("Current value: 0x%04x\n", current_value);
    ImGui::Text("Target value: 0x%04x\n", target_value);

    auto sync_enable = counter_mode & 1;
    auto sync_mode = (counter_mode >> 1) & 3;
    auto reset_counter = (counter_mode >> 3) & 1;
    auto irq_when_target = (counter_mode >> 4) & 1;
    auto irq_when_ffff = (counter_mode >> 5) & 1;
    auto irq_repeat_mode = (counter_mode >> 6) & 1;
    auto irq_pulse_or_toggle = (counter_mode >> 7) & 1;
    auto clock_source = (counter_mode >> 8) & 3;

    ImGui::Text("Sync mode: ");
    ImGui::SameLine();
    if (sync_enable == 0) ImGui::Text("Off (Free run)\n");
    else if constexpr (timer == 0) {
        if (sync_mode == 0) ImGui::Text("Pause during Hblank\n");
        else if (sync_mode == 1) ImGui::Text("Reset to 0 at Hblank\n");
        else if (sync_mode == 2) ImGui::Text("Pause outside Hblank and reset to 0 at the start of Hblank\n");
        else if (sync_mode == 3) ImGui::Text("Pause until the next Hblank, then switch to Free run\n");
    }
    else if constexpr (timer == 1) {
        if (sync_mode == 0) ImGui::Text("Pause during Vblank\n");
        else if (sync_mode == 1) ImGui::Text("Reset to 0 at Vblank\n");
        else if (sync_mode == 2) ImGui::Text("Pause outside Vblank and reset to 0 at the start of Vblank\n");
        else if (sync_mode == 3) ImGui::Text("Pause until the next Vblank, then switch to Free run\n");
    }
    else if constexpr (timer == 2) {
        if ((sync_mode == 0) || (sync_mode == 3)) ImGui::Text("Stopped\n");
        else if ((sync_mode == 1) || (sync_mode == 2)) ImGui::Text("Off (Free run\n");
    }

    ImGui::Text("Reset counter to 0 if: ");
    ImGui::SameLine();
    if (reset_counter == 0) ImGui::Text("Counter hits 0xFFFF\n");
    else if (reset_counter == 1) ImGui::Text("Counter hits target\n");

    ImGui::Text("Fire IRQ if: ");
    ImGui::SameLine();
    if (!irq_when_target && !irq_when_ffff) ImGui::Text("Never\n");
    else {
        if (irq_when_target) ImGui::Text("counter==target");
        if (irq_when_ffff) {
            if (!irq_when_target) ImGui::Text("counter==0xFFFF\n");
            else ImGui::Text(" or counter==0xFFFF\n");
        }
    }

    ImGui::Text("Fire IRQ repeatedly: ");
    ImGui::SameLine();
    if (irq_repeat_mode) ImGui::Text("Yes\n");
    else ImGui::Text("No\n");

    ImGui::Text("Interrupt Request bit pulse/toggle: ");
    ImGui::SameLine();
    if (irq_pulse_or_toggle) ImGui::Text("Toggle\n");
    else ImGui::Text("Pulse\n");

    ImGui::Text("Clock source: ");
    ImGui::SameLine();
    if constexpr (timer == 0) {
        if ((clock_source == 0) || (clock_source == 2)) ImGui::Text("System Clock\n");
        else if ((clock_source == 1) || (clock_source == 3)) ImGui::Text("Dotclock\n");
    }
    else if constexpr (timer == 1) {
        if ((clock_source == 0) || (clock_source == 2)) ImGui::Text("System Clock\n");
        else if ((clock_source == 1) || (clock_source == 3)) ImGui::Text("Hblank\n");
    }
    else if constexpr (timer == 2) {
        if ((clock_source == 0) || (clock_source == 1)) ImGui::Text("System Clock\n");
        else if ((clock_source == 2) || (clock_source == 3)) ImGui::Text("System Clock/8\n");
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
    ImVec2 uv0 = ImVec2(Cpu->bus.Gpu.drawing_topleft_x / 1024.f, Cpu->bus.Gpu.drawing_topleft_y / 512.f);
    ImVec2 uv1 = ImVec2(Cpu->bus.Gpu.drawing_bottomright_x / 1024.f, Cpu->bus.Gpu.drawing_bottomright_y / 512.f);
    ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(Cpu->bus.Gpu.VramTexture)), image_size, uv0, uv1);
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
                    Cpu->bus.mem.CDROM.cd.OpenFile(game_path);
                }
            }
    
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
    
        if (ImGui::BeginMenu("Emulation")) {
            if (ImGui::MenuItem("Start", NULL, false, !started)) {
                Reset("", bios_path.c_str(), Cpu);
            }
    
            if (ImGui::MenuItem(run ? "Pause" : "Resume", NULL, false, started)) {
                run = !run;
            }
    
            if (ImGui::MenuItem("Reset")) {
                Reset("", bios_path.c_str(), Cpu);
                run = false;
                started = false;
            }
    
            ImGui::EndMenu();
        }
    
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("System")) {
                show_system_settings = true;
            }
            if (ImGui::MenuItem("Configure Pads")) {
                show_pad_settings = true;
            }
            ImGui::EndMenu();
        }
    
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("CPU")) {
                show_cpu_registers = !show_cpu_registers;
            }
            if (ImGui::BeginMenu("GPU")) {
                if (ImGui::MenuItem("Show shader editor")) show_shader_editor = !show_shader_editor;
                if (ImGui::MenuItem("Show textured primitives shader editor")) show_textured_primitives_shader_editor = !show_textured_primitives_shader_editor;
                ImGui::EndMenu();
            }
#ifdef log_cpu
            if (ImGui::MenuItem("CPU Trace")) {
                Cpu->debug = !Cpu->debug;
            }
#endif
            if (ImGui::MenuItem("Interrupts")) {
                show_interrupt_debugger = !show_interrupt_debugger;
            }
            if (ImGui::BeginMenu("Timers")) {
                if (ImGui::MenuItem("Timer 0")) {
                    show_timer0_debugger = !show_timer0_debugger;
                }
                if (ImGui::MenuItem("Timer 1")) {
                    show_timer1_debugger = !show_timer1_debugger;
                }
                if (ImGui::MenuItem("Timer 2")) {
                    show_timer2_debugger = !show_timer2_debugger;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Log")) {
                show_log = !show_log;
            }
            if (ImGui::MenuItem("VRAM Viewer")) {
                show_vram = !show_vram;
            }
            if (ImGui::BeginMenu("Memory Viewer")) {
                if (ImGui::MenuItem("RAM")) {
                    show_ram_viewer = !show_ram_viewer;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Dump RAM")) {
                std::ofstream file("ram.bin", std::ios::binary);
                file.write((const char*)Cpu->bus.mem.ram, 0x200000);
            }
            if (ImGui::MenuItem("test")) {
                test = !test;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static const GLchar* VertexShaderSource =
R"(
#version 330 core
precision highp float;
in vec2 position;
in vec2 UV;
uniform mat4 projMatrix;

out vec2 fragUV;

void main() {
    fragUV = UV;
    gl_Position = projMatrix * vec4(position.xy, 0.f, 1.f);
}
)";
static const GLchar* FragmentShaderSource =
R"(
#version 330 core
precision highp float;
uniform sampler2D _texture;
in vec2 fragUV;
layout (location = 0) out vec4 outColour;

void main() {
    vec4 c = texture(_texture, fragUV.st);
    c.a = 1.f;
    outColour = c;
} 
)";

GLuint ShaderProgram;
GLuint vram_texture;
void ImGuiCallback_VRAM(const ImDrawList* parentList, const ImDrawCmd* cmd) {
    GLfloat projMtx[4][4];
    GLint imguiProgramID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &imguiProgramID);
    GLint projmtxloc = glGetUniformLocation(imguiProgramID, "ProjMtx");
    //printf("%d\n", projmtxloc);
    glGetUniformfv(imguiProgramID, projmtxloc, &projMtx[0][0]);
    glUseProgram(ShaderProgram);
    projmtxloc = glGetUniformLocation(ShaderProgram, "projMatrix");
    //printf("%d\n", projmtxloc);
    glUniformMatrix4fv(projmtxloc, 1, GL_FALSE, &projMtx[0][0]);
    //GLuint textureID = static_cast<GLuint>(reinterpret_cast<uintptr_t>(cmd->TextureId));
    //glBindTexture(GL_TEXTURE_2D, vram_texture);
    //GLint textureloc = glGetUniformLocation(ShaderProgram, "_texture");
    //glUniform1i(textureloc, 0);
    //printf("text %d\n", vram_texture);
}

void UpdateVRAMViewer(cpu* Cpu) {
    uint32_t* pixels = new uint32_t[1024 * 512];
    glBindTexture(GL_TEXTURE_2D, Cpu->bus.Gpu.VramTexture);
    glGetTextureImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 1024 * 512 * 4, pixels);
    glBindTexture(GL_TEXTURE_2D, vram_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
}

void RenderVRAMViewer(cpu* Cpu) {
    ImGui::Begin("Imagevram", NULL, ImGuiWindowFlags_NoTitleBar);

    float x = ImGui::GetWindowSize().x - 15, y = ImGui::GetWindowSize().y - 15;
    float current_aspect_ratio = x / y;
    if (vram_aspect_ratio > current_aspect_ratio) {
        y = x / vram_aspect_ratio;
    }
    else {
        x = y * vram_aspect_ratio;
    }


    //UpdateVRAMViewer(Cpu);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 image_size(x, y);
    ImVec2 centered((ImGui::GetWindowSize().x - image_size.x) * 0.5, (ImGui::GetWindowSize().y - image_size.y) * 0.5);
    ImGui::SetCursorPos(centered);
    drawList->AddCallback(ImGuiCallback_VRAM, nullptr);
    vram_texture = Cpu->bus.Gpu.VramTexture;
    ImGui::Image(reinterpret_cast<ImTextureID*>(vram_texture), image_size);
    drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    ImGui::End();
}
int main(int argc, char** argv) {
    /*struct Code : Xbyak::CodeGenerator {
        Code(int x)
        {
            mov(eax, x);
            ret();
        }
    };

    Code c(5);
    int (*f)() = c.getCode<int (*)()>();
    printf("ret=%d\n", f()); // ret = 5

    exit(1);*/

    if (!glfwInit()) {
        exit(1);
    }
    printf("\n Executing \n \n");
    // Parse CLI args (TODO: Use a library)
    const auto rom_dir = argc > 1 ? std::string(argv[1]) : "";  // Path of the ROM (Or "" if we just want to run the BIOS)
    const bool running_in_ci = argc > 2 && std::string(argv[2]).compare("--continuous-integration") == 0; // Running in CI makes the system run without SDL 
    const std::string bios_dir = running_in_ci ? "" : "./SCPH1001.bin"; // In CI, don't load a BIOS, otherwise load SCPH1001.bin. TODO: Add a CLI arg for the BIOS path

    cpu Cpu = cpu(rom_dir, bios_dir, running_in_ci); // TODO: Have a system class, don't use the CPU as one   
    if (running_in_ci) { // When executing in CI, run the system headless, without a BIOS.
        Cpu.sideloadExecutable(rom_dir);
        while (true) // The CI tests write to a custom register that force-closes the emulator when they're done, so this will run until that is hit
            Cpu.step();
    }

    InitWindow();
    Cpu.bus.Gpu.InitGL();
    glGenTextures(1, &vram_texture);
    glBindTexture(GL_TEXTURE_2D, vram_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1024, 512);

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);
    int success;
    char InfoLog[512];
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
        std::cout << "VRAM Vertex shader compilation failed\n" << InfoLog << std::endl;
    }
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(FragmentShader);
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(FragmentShader, 512, NULL, InfoLog);
        std::cout << "VRAM Fragment shader compilation failed\n" << InfoLog << std::endl;
    }
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ShaderProgram, 512, NULL, InfoLog);
        std::cout << "Linking shader program failed\n" << InfoLog << std::endl;
    }
    
    ShaderEditor ShaderEditUntextured = { "untextured" };
    ShaderEditor ShaderEditTextured = { "textured" };
    ShaderEditUntextured.init();
    ShaderEditTextured.init();
    ShaderEditUntextured.setText(Cpu.bus.Gpu.VertexShaderSource, Cpu.bus.Gpu.FragmentShaderSource, "");
    ShaderEditTextured.setText(Cpu.bus.Gpu.TextureVertexShaderSource, Cpu.bus.Gpu.TextureFragmentShaderSource, "");
    ShaderEditUntextured.m_show = true;
    ShaderEditTextured.m_show = true;

    double prevTime = glfwGetTime();
    int frameCount = 0;
    while (!glfwWindowShouldClose(window)) {
        if (Cpu.frame_cycles >= (33868800 / 60) || !run) {
            if(run) Cpu.bus.mem.I_STAT |= 1;
            if (tmr1irq) {
                printf("[TIMERS] TMR2 IRQ\n");
                //tmr1irq = false;
                //Cpu.bus.mem.I_STAT |= 0b100000;
                Cpu.bus.mem.I_STAT |= 0b1000000;
                Cpu.bus.mem.tmr1_stub = 0;
            }
            if (spuirq) {
                printf("[SPU] IRQ\n");
                Cpu.bus.mem.I_STAT |= (1 << 9);
                //spuirq = false;
            }
            if (padirq) {
                printf("[PAD] IRQ\n");
                Cpu.bus.mem.I_STAT |= (1 << 7);
            }
            Cpu.bus.mem.I_STAT |= 0b100000;
            //Cpu.bus.mem.I_STAT |= 0b1000000;
            if (Cpu.should_service_dma_irq) {
                //printf("[DMA] IRQ\n");
                //Cpu.bus.mem.I_STAT |= 0b1000;
                //Cpu.should_service_dma_irq = false;
            }
            //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if (pad1_source == "Gamepad") {
                GLFWgamepadstate state;
                P1buttons = 0xffff;
                if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
                        P1buttons &= ~(0b0100000000000000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS) {
                        P1buttons &= ~(0b1000000000000000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
                        P1buttons &= ~(0b0010000000000000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS) {
                        P1buttons &= ~(0b0001000000000000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS) {
                        P1buttons &= ~(0b0000000000010000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS) {
                        P1buttons &= ~(0b0000000000100000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS) {
                        P1buttons &= ~(0b0000000001000000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS) {
                        P1buttons &= ~(0b0000000010000000);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE] == GLFW_PRESS) {
                        P1buttons &= ~(0b0000000000000001);
                    }
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS) {
                        printf("start\n");
                        P1buttons &= ~(0b0000000000001000);
                    }
                }
            }
            if (pad2_source == "Gamepad") {
                GLFWgamepadstate state;
                P2buttons = 0xffff;
                if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
                {
                    //printf("aaa\n");
                    if (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS)
                    {
                        //printf("aaa\n");
                        P2buttons &= ~(0b0100000000000000);
                    }
                }
            }
            Cpu.bus.mem.pads.P1buttons = P1buttons;
            Cpu.bus.mem.pads.P2buttons = P2buttons;
            Cpu.bus.mem.pads.pad1_type = pad1_type;
            Cpu.bus.mem.pads.pad2_type = pad2_type;
            Cpu.bus.mem.pads.pad1_connected = pad1_connected;
            Cpu.bus.mem.pads.pad2_connected = pad2_connected;
            if (Cpu.patch_b0_12h) {
                if (pad1_connected) {
                    if (pad1_type == "Digital") {
                        Cpu.bus.mem.write32(Cpu.button_dest, P1buttons | (0x41 << 16));
                        Cpu.bus.mem.write(Cpu.button_dest + 0, 0x00, false);
                        Cpu.bus.mem.write(Cpu.button_dest + 1, 0x41, false);
                        Cpu.bus.mem.write(Cpu.button_dest + 2, (P1buttons & 0xff), false);
                        Cpu.bus.mem.write(Cpu.button_dest + 3, ((P1buttons >> 8) & 0xff), false);
                    }
                    else if (pad1_type == "Mouse") {
                        auto& io = ImGui::GetIO();
                        int leftClick = ImGui::IsMouseDown(ImGuiMouseButton_Left) ? 0 : 1;
                        int rightClick = ImGui::IsMouseDown(ImGuiMouseButton_Right) ? 0 : 1;
                        uint16_t buttons = (leftClick << 1) | rightClick;
                        Cpu.bus.mem.write(Cpu.button_dest + 0, 0x00, false);
                        Cpu.bus.mem.write(Cpu.button_dest + 1, 0x12, false);
                        Cpu.bus.mem.write(Cpu.button_dest + 2, 0xff, false);
                        Cpu.bus.mem.write(Cpu.button_dest + 3, 0xf0 | (buttons << 2), false);
                        Cpu.bus.mem.write(Cpu.button_dest + 4, io.MouseDelta.x, false);
                        Cpu.bus.mem.write(Cpu.button_dest + 5, io.MouseDelta.y, false);
                    }
                } else Cpu.bus.mem.write(Cpu.button_dest, 0xff, false);
            }
            glfwPollEvents();
            //if (run) Cpu.bus.mem.I_STAT |= 1;
            // Update the ImGui frontend
            ImGuiFrame(&Cpu);
            if (show_vram) RenderVRAMViewer(&Cpu);
            if (show_system_settings) SystemSettingsMenu(&Cpu);
            if (show_pad_settings) PadSettingsMenu();
            if (show_dialog) Dialog();
            if (show_ram_viewer) MemEditor.DrawWindow("RAM Viewer", Cpu.bus.mem.ram, 0x200000);
            if (show_cpu_registers) CpuDebugger(&Cpu);
            if (show_interrupt_debugger) InterruptDebugger(&Cpu);
            if (show_timer0_debugger) TimerDebugger<0>(&Cpu);
            if (show_timer1_debugger) TimerDebugger<1>(&Cpu);
            if (show_timer2_debugger) TimerDebugger<2>(&Cpu);
            if (show_log) Cpu.log.Draw("Log");
            if (show_shader_editor) {
                auto programuntextured = ShaderEditUntextured.compile();
                if (programuntextured.has_value()) Cpu.bus.Gpu.ShaderProgram = programuntextured.value();
                ShaderEditUntextured.draw("Untextured primitives shader editor");
            }
            if (show_textured_primitives_shader_editor) {
                auto programtextured = ShaderEditTextured.compile();
                if (programtextured.has_value()) Cpu.bus.Gpu.TextureShaderProgram = programtextured.value();
                ShaderEditTextured.draw("Textured primitives shader editor");
            }

            // Render it
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            
            Cpu.frame_cycles = 0;

            // Update the FPS
            frameCount++;
            double currTime = glfwGetTime();
            if (currTime - prevTime >= 1.0) {
                std::stringstream title;
                title << "ChonkyStation " << "[" << frameCount << " FPS]";
                glfwSetWindowTitle(window, title.str().c_str());
                frameCount = 0;
                prevTime = currTime;
            }
            //Cpu.bus.Gpu.ClearScreen();
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            if (test) printf("%x\n", Cpu.pc);
            Cpu.step();
        }   
    }

    return 0;
}