/* date = August 28th 2020 2:39 pm */
#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"

static void gui_init();
static void gui_begin_frame();
static void gui_end_frame();
static void gui_tick();

// Widgets
static void gui_show_debug_info(app *application);