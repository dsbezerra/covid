
#include <math.h>
#include <windows.h>
#include <winhttp.h>
#include <GL/gl3w.h>
#include "wglext.h"

#include "covid.cpp"

#include "win32_covid.h"

#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"

global_variable app *application;
global_variable win32_http_state http_state = {0};

global_variable u8 *page_url = (u8 *) "saude.montesclaros.mg.gov.br";

// This should be enough to store our page.
global_variable char global_body_buffer[2048 * 2048];

global_variable LONGLONG global_perf_count_frequency;

internal inline LARGE_INTEGER
win32_get_wallclock(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

internal inline real32
win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    real32 result = ((real32) (end.QuadPart - start.QuadPart) /
                     (real32) global_perf_count_frequency);
    return result;
}

internal void
winhttp_close() {
    if (WinHttpCloseHandle(http_state.request)) {
        http_state.request = 0;
    }
    if (WinHttpCloseHandle(http_state.session)) {
        http_state.session = 0;
    }
    if (WinHttpCloseHandle(http_state.connect)) {
        http_state.connect = 0;
    }
}

internal void
winhttp_status_callback(HINTERNET internet,
                        DWORD_PTR context,
                        DWORD internet_status,
                        LPVOID status_information,
                        DWORD status_information_length) {
    
    
    HINTERNET request = http_state.request;
    
    switch (internet_status) {
        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: {
            if (request && WinHttpReceiveResponse(request, 0)) {}
        } break;
        
        case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
            app_set_state(application, State_ReadingPage);
            
            DWORD size = *((DWORD *) status_information);
            if (size) {
                // @Cleanup temporary allocation
                LPSTR buffer = new char[size + 1];
                ZeroMemory(buffer, size + 1);
                
                DWORD read;
                if (!WinHttpReadData(request, buffer, size, &read)) {
                    // TODO(diego): Diagnostic
                }
            }
        } break;
        
        case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: {
            if (status_information_length == 0) {
                return;
            }
            
            strcat(global_body_buffer, (char *) status_information);
            
            delete status_information;
            
            DWORD size;
            if (WinHttpQueryDataAvailable(request, &size)) {
            }
            
            if (size == 0) {
                int len = string_length(global_body_buffer);
                server_response *page = create_server_response(global_body_buffer, len);
                ZeroMemory(global_body_buffer, len  + 1);
                
                app_set_state(application, State_ParsingPage);
                app_parse_page(application, page, page_url);
                
                destroy_server_response(page);
                winhttp_close();
            }
            
        } break;
        
        case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE: {
            app_set_state(application, State_ReceivingResponse);
        } break;
        
        case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED: {
            app_set_state(application, State_ResponseReceived);
        } break;
        
        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
            DWORD size;
            if (WinHttpQueryDataAvailable(request, &size)) {}
        } break;
        
        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
            app_set_state(application, State_NetworkError);
        } break;
        
        case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING: {
            WinHttpSetStatusCallback(http_state.request, 0, 0, 0);
        } break;
        
        default: {
            // No-op
        } break;
    }
}

internal void
winhttp_async_get(char *url) {
    app_set_state(application, State_OpeningConnection);
    http_state.session = WinHttpOpen(L"Default", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    if (http_state.session) {
        WinHttpSetStatusCallback(http_state.session, winhttp_status_callback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
        
        int url_size = string_length(url) + 1;
        wchar_t *r = new wchar_t[url_size];
        mbstowcs(r, url, url_size);
        
        http_state.connect = WinHttpConnect(http_state.session, r, INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (http_state.connect) {
            http_state.request = WinHttpOpenRequest(http_state.connect, L"GET", 0, 0, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            // If request was successful then we retrieve the response.
            if (http_state.request && WinHttpSendRequest(http_state.request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                // NOTE(diego): Async
            } else {
                winhttp_close();
            }
        } else {
            // TODO(diego): Diagnostic
            app_set_state(application, State_NetworkError);
            WinHttpCloseHandle(http_state.session);
        }
        
        delete r;
        
    } else {
        // TODO(diego): Diagnostic
        app_set_state(application, State_NetworkError);
    }
}

internal void
win32_opengl_get_functions() {
    assert(open_gl);
    
    opengl_get_function(wglSwapIntervalEXT);
}

internal void
win32_init_opengl(HWND window) {
    
    open_gl = (opengl *) VirtualAlloc(0, sizeof(opengl), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    HDC window_dc = GetDC(window);
    
    PIXELFORMATDESCRIPTOR pixel_format_descriptor = {};
    pixel_format_descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format_descriptor.nVersion = 1;
    pixel_format_descriptor.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pixel_format_descriptor.cColorBits = 32;
    pixel_format_descriptor.cAlphaBits = 8;
    
    int pixel_format_index = ChoosePixelFormat(window_dc, &pixel_format_descriptor);
    
    PIXELFORMATDESCRIPTOR suggested_pixel_format = {};
    if (DescribePixelFormat(window_dc, pixel_format_index,
                            sizeof(suggested_pixel_format), &suggested_pixel_format)) {
        if (SetPixelFormat(window_dc, pixel_format_index, &suggested_pixel_format)) {
            HGLRC gl_rc = wglCreateContext(window_dc);
            if (wglMakeCurrent(window_dc, gl_rc)) {
                win32_opengl_get_functions();
                open_gl->wglSwapIntervalEXT(1);
                gl3wInit();
                open_gl->info = opengl_get_info();
            } else {
                invalid_code_path;
            }
        } else {
            invalid_code_path;
        }
        
    } else {
        invalid_code_path;
    }
    
    ReleaseDC(window, window_dc);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

internal LRESULT CALLBACK
default_proc(HWND window,
             UINT message,
             WPARAM wparam,
             LPARAM lparam) {
    
    if (ImGui_ImplWin32_WndProcHandler(window, message, wparam, lparam))
        return true;
    
    LRESULT result = 0;
    switch (message) {
        case WM_DESTROY: {
            application->running = false;
        } break;
        case WM_CLOSE: {
            application->running = false;
        };
        case WM_SIZE: {
            assert(application);
            RECT rect;
            GetClientRect(window, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            application->width = width;
            application->height = height;
        } break;
        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        } break;
    }
    return result;
}

int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR command_line,
        int show_code) {
    
    AllocConsole();
    freopen("CONOUT$", "w+", stdout);
    
    LARGE_INTEGER perf_count_freq_res;
    QueryPerformanceFrequency(&perf_count_freq_res);
    global_perf_count_frequency = perf_count_freq_res.QuadPart;
    
    
    bool sleep_is_granular = timeBeginPeriod(1) == TIMERR_NOERROR; 
    
    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = default_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "CovidWindowClass";
    
    
    if (RegisterClassA(&window_class)) {
        
        int monitor_hz = 120;
        int app_update_hz = monitor_hz;
        real32 target_seconds_per_frame = 1.f / (real32) app_update_hz;
        
        application = app_init();
        application->dt = target_seconds_per_frame;
        application->ms_per_frame_to_draw = target_seconds_per_frame;
        application->fps_to_draw = monitor_hz;
        application->current_time = target_seconds_per_frame;
        application->frame_time_render_rate = init_interval(1.f / 1000.f);
        
        HWND window = CreateWindowExA(0,
                                      window_class.lpszClassName,
                                      "COVID-19 - Montes Claros",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      application->width,
                                      application->height,
                                      0,
                                      0,
                                      instance,
                                      0);
        if (window) {
            HDC hdc = GetDC(window);
            
            win32_init_opengl(window);
            app_init_fonts(application);
            
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
            ImGui_ImplWin32_Init(window);
            ImGui_ImplOpenGL3_Init();
            
            app_init_gui(application);
            
            app_set_clear_color(make_color(0x0));
            
            LARGE_INTEGER frequency_counter_large;
            QueryPerformanceFrequency(&frequency_counter_large);
            real32 frequency_counter = (real32) frequency_counter_large.QuadPart;
            
            LARGE_INTEGER last_counter;
            QueryPerformanceCounter(&last_counter);
            
            immediate_init();
            init_shaders();
            set_shader(global_shader);
            
            
            while (application->running) {
                
                MSG message;
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    
                    switch (message.message) {
                        case WM_SYSKEYDOWN:
                        case WM_SYSKEYUP:
                        case WM_KEYDOWN:
                        case WM_KEYUP: {
                            u32 vk_code = (u32) message.wParam;
                            bool was_down = ((message.lParam & (1 << 30)) != 0);
                            bool is_down = ((message.lParam & (1UL << 31)) == 0);
                            if (was_down != is_down) {
                                switch (vk_code) {
                                    case 'D': {
                                        if (is_down) {
                                            application->gui.show_debug_info = !application->gui.show_debug_info;
                                        }
                                    } break;
                                    
                                    case 'U': {
                                        if (!http_state.request && is_down) {
                                            winhttp_async_get((char *) page_url);
                                        }
                                    } break;
                                    
                                    default: break;
                                }
                            }
                            
                            bool alt_key_was_down = (message.lParam & (1 << 29));
                            if ((vk_code == VK_F4) && alt_key_was_down) {
                                application->running = false;
                            }
                            
                        } break;
                        
                        default: {
                            TranslateMessage(&message);
                            DispatchMessageA(&message);
                        } break;
                    }
                }
                
                // Update 
                app_update(application);
                app_draw(application);
                app_gui_tick(application);
                
                // Ensure a forced frame time
                LARGE_INTEGER work_counter = win32_get_wallclock();
                real32 work_seconds_elapsed = win32_get_seconds_elapsed(last_counter, work_counter);
                
                real32 seconds_elapsed_for_frame = work_seconds_elapsed;
                if (seconds_elapsed_for_frame < target_seconds_per_frame) {
                    if (sleep_is_granular) {
                        DWORD sleep_ms = (DWORD)(1000.0f * (target_seconds_per_frame -
                                                            seconds_elapsed_for_frame));
                        if (sleep_ms > 0) {
                            Sleep(sleep_ms);
                        }
                    }
                    
                    while (seconds_elapsed_for_frame < target_seconds_per_frame) {
                        seconds_elapsed_for_frame= win32_get_seconds_elapsed(last_counter,
                                                                             win32_get_wallclock());
                    }
                }
                
                SwapBuffers(hdc);
                
                // Get the frame time
                LARGE_INTEGER end_counter = win32_get_wallclock();
                real32 ms_per_frame = 1000.0f * win32_get_seconds_elapsed(last_counter, end_counter);
                int fps = (int) (global_perf_count_frequency / (end_counter.QuadPart - last_counter.QuadPart));
                
                application->dt = ms_per_frame;
                application->current_time += ms_per_frame;
                application->fps = fps;
                last_counter = end_counter;
            }
            
            immediate_free();
            app_free(application);
            
            ReleaseDC(window, hdc);
        }
    }
    
    return 0;
}