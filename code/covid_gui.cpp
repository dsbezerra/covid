#include "covid_gui.h"

static void
gui_init(app *application) {
    application->gui.show_debug_info = true;
    application->gui.show_demo_window = true;
    
    // Setup style
    ImGui::StyleColorsDark();
}

static void
gui_begin_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

static void
gui_tick(app *application) {
    gui_state state = application->gui;
    
    gui_begin_frame();
    
    if (state.show_debug_info) {
        gui_show_debug_info(application);
    }
    
    gui_end_frame();
}

static void
gui_end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void
gui_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

//
// Widgets
//
static void
gui_show_debug_info(app *application) {
    const float DISTANCE = 10.0f;
    
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_pos = ImVec2(io.DisplaySize.x - DISTANCE, DISTANCE);
    ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    
    // Transparent background
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("Debug Info", &application->gui.show_debug_info, window_flags))
    {
        char debug_buffer[256];
        opengl_info info = open_gl->info;
        
        int fps = application->fps_to_draw;
        real32 ms_per_frame = application->ms_per_frame_to_draw;
        
        _snprintf_s(debug_buffer, sizeof(debug_buffer),
                    "%s\n%s\n%s\n%.02f ms, %d fps", info.vendor, info.renderer, info.version, ms_per_frame, fps);
        
        
        ImGui::Text(debug_buffer);
    }
    
    ImGui::End();
}