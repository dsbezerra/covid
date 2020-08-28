#include "covid_gui.h"

void
gui_init(app *application) {
    application->gui.show_debug_info = true;
    application->gui.show_demo_window = true;
    
    // Setup style
    ImGui::StyleColorsDark();
}

void
gui_begin_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void
gui_end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
gui_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}