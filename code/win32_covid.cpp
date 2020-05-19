#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <winhttp.h>
#include <gl/gl.h>

#include "covid.cpp"
#include "covid_opengl.cpp"

#include "win32_covid.h"
#include "common.cpp"
#include "font.cpp"

#include "shader.cpp"
#include "draw.cpp" 

app application = init_app();
win32_http_state http_state = {0};

char *page_url = "saude.montesclaros.mg.gov.br";

// This should be enough to store our page.
char global_body_buffer[2048 * 2048];

LONGLONG global_perf_count_frequency;

inline LARGE_INTEGER
win32_get_wallclock(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

inline real32
win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    real32 result = ((real32) (end.QuadPart - start.QuadPart) /
                     (real32) global_perf_count_frequency);
    return result;
}

static void
winhttp_close() {
    WinHttpCloseHandle(http_state.request);
    WinHttpCloseHandle(http_state.session);
    WinHttpCloseHandle(http_state.connect);
}

static void
winhttp_status_callback(HINTERNET internet,
                        DWORD_PTR context,
                        DWORD internet_status,
                        LPVOID status_information,
                        DWORD status_information_length) {
    
    
    HINTERNET request = http_state.request;
    
    switch (internet_status) {
        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: {
            if (request && WinHttpReceiveResponse(request, 0)) {
            }
        } break;
        
        case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
            app_set_state(&application, State_ReadingPage);
            
            DWORD size = *((DWORD *) status_information);
            if (size) {
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
                server_response *page = create_server_response(global_body_buffer, strlen(global_body_buffer));
                ZeroMemory(global_body_buffer, strlen(global_body_buffer) + 1);
                
                app_set_state(&application, State_ParsingPage);
                parse_page(&application, page);
                delete page;
                
            }
            
        } break;
        
        case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE: {
            app_set_state(&application, State_ReceivingResponse);
        } break;
        
        case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED: {
            app_set_state(&application, State_ResponseReceived);
        } break;
        
        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
            DWORD size;
            if (WinHttpQueryDataAvailable(request, &size)) {
            }
        } break;
        
        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
            app_set_state(&application, State_NetworkError);
        } break;
        
        default: {
            // No-op
        } break;
    }
}

static void
winhttp_async_get(char *url) {
    app_set_state(&application, State_OpeningConnection);
    http_state.session = WinHttpOpen(L"Default", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    if (http_state.session) {
        WinHttpSetStatusCallback(http_state.session, winhttp_status_callback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
        
        int url_size = strlen(url) + 1;
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
            app_set_state(&application, State_NetworkError);
            WinHttpCloseHandle(http_state.session);
        }
        
        delete r;
        
    } else {
        // TODO(diego): Diagnostic
        app_set_state(&application, State_NetworkError);
    }
}

static void
win32_init_opengl(HWND window) {
    
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


static LRESULT CALLBACK
default_proc(HWND window,
             UINT message,
             WPARAM wparam,
             LPARAM lparam) {
    LRESULT result = 0;
    switch (message) {
        case WM_DESTROY: {
            application.running = false;
        } break;
        case WM_CLOSE: {
            application.running = false;
        };
        case WM_SIZE: {
            RECT rect;
            GetClientRect(window, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            if (app_resize(&application, width, height)) {
                resize_viewport(width, height);
            }
        } break;
        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        } break;
    }
    return result;
}

struct timer_interval {
    real32 current_ms;
    real32 interval_ms;
};

struct switchable_color {
    int current;
    union {
        struct {
            v4 a;
            v4 b;
        };
        v4 e[2];
    };
};

static void
switch_colors(switchable_color *sc) {
    if (sc->current == 0) {
        sc->current = 1;
    } else {
        sc->current = 0;
    }
}

static switchable_color
create_switchable_color(v4 a, v4 b) {
    switchable_color result = {};
    result.a = a;
    result.b = b;
    result.current = 0;
    return result;
}

static timer_interval
init_interval(real32 seconds) {
    timer_interval result = {};
    result.current_ms = 0.f;
    result.interval_ms = seconds * 1000.0f;
    return result;
}

static int
timer_increment(timer_interval *interval, real32 dt) {
    if (!interval) return 0;
    
    interval->current_ms += dt;
    if (interval->current_ms >= interval->interval_ms) {
        interval->current_ms = 0.f;
        return 1;
    }
    
    return 0;
}

int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR command_line,
        int show_code) {
    
    LARGE_INTEGER perf_count_freq_res;
    QueryPerformanceFrequency(&perf_count_freq_res);
    global_perf_count_frequency = perf_count_freq_res.QuadPart;
    
    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = default_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "CovidWindowClass";
    
    int monitor_hz = 60;
    real32 target_seconds_per_frame = 1.f / monitor_hz;
    
    if (RegisterClassA(&window_class)) {
        HWND window = CreateWindowExA(0,
                                      window_class.lpszClassName,
                                      "COVID-19 - Montes Claros",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      application.width,
                                      application.height,
                                      0,
                                      0,
                                      instance,
                                      0);
        if (window) {
            HDC hdc = GetDC(window);
            
            win32_init_opengl(window);
            
            font debug_font = my_stbtt_initfont("c:/windows/fonts/LiberationMono-Regular.ttf", 16.f);
            font label_font = my_stbtt_initfont("c:/windows/fonts/LiberationMono-Regular.ttf", 24.f);
            font value_font = my_stbtt_initfont("c:/windows/fonts/LiberationMono-Bold.ttf", 24.f);
            
            LARGE_INTEGER frequency_counter_large;
            QueryPerformanceFrequency(&frequency_counter_large);
            real32 frequency_counter = (real32) frequency_counter_large.QuadPart;
            
            LARGE_INTEGER last_counter;
            QueryPerformanceCounter(&last_counter);
            
            timer_interval *change_clear_color_interval = 0;
            
            switchable_color clear_color = create_switchable_color(make_color(0.f, 0.f, 0.f), make_color(0.5f, 0.7f, 0.5f));
            
            application.current_time = target_seconds_per_frame;
            
            init_quad_objects();
            resize_viewport(application.width, application.height);
            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            while (application.running) {
                
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
                                if (vk_code == 'U' && !http_state.request) {
                                    winhttp_async_get(page_url);
                                }
                                if (vk_code == 'I' && is_down) {
                                    if (change_clear_color_interval) {
                                        change_clear_color_interval = 0;
                                    } else {
                                        timer_interval r = init_interval(0.5f);
                                        change_clear_color_interval = &r;
                                    }
                                }
                            }
                            
                            bool alt_key_was_down = (message.lParam & (1 << 29));
                            if ((vk_code == VK_F4) && alt_key_was_down) {
                                application.running = false;
                            }
                            
                        } break;
                        
                        default: {
                            TranslateMessage(&message);
                            DispatchMessageA(&message);
                        } break;
                    }
                }
                
                // Defines clear color and clear
                v4 my_color = clear_color.e[clear_color.current];
                glClearColor(my_color.r,
                             my_color.g,
                             my_color.b,
                             my_color.a);
                glClear(GL_COLOR_BUFFER_BIT);
                
                real32 margin = application.width * 0.025f;
                
                int state = application.current_state;
                switch (state) {
                    case State_Processed: {
                        real32 ypos = margin;
                        real32 card_height = (real32) (application.height - 2 * margin) / (real32) array_count(application.cards);
                        for (int i = 0; i < array_count(application.cards); i++) {
                            card *card = get_card(&application, i);
                            if (!card || card->kind == Card_None) {
                                continue;
                            }
                            draw_quad(margin, ypos, (real32) application.width - margin, ypos + card_height, card->background_color);
                            
                            // @Incomplete reset shader so we can render our text for now.
                            set_shader(0);
                            
                            real32 card_margin = card_height * 0.1f;
                            real32 card_center_y = card_height * 0.4f + 6.f;
                            my_stbtt_print(&label_font, margin*2.f, ypos + card_center_y, card->label, make_color(1.f, 1.f, 1.f));
                            
                            char value[256];
                            wsprintf(value, "%d", card->value);
                            my_stbtt_print(&value_font, margin*2.f, ypos + card_center_y + 24.f, (u8*) value, make_color(1.f, 1.f, 1.f));
                            
                            ypos += card_height;
                        }
                    } break;
                    
                    default: {
                        if (state >= 0 && state < State_Count) {
                            char *desc = app_get_state_description(&application);
                            my_stbtt_print(&value_font, margin, application.height / 2.0f, (u8*) desc, make_color(1.f, 1.f, 1.f));
                            
                            switch (state) {
                                case State_NetworkError: {
                                    my_stbtt_print(&label_font, margin, (application.height / 2.0f) + 24.f, (u8 *) page_url, make_color(1.f, 0.f, 0.f));
                                } break;
                                
                                default: {
                                } break;
                            }
                        }
                    };
                }
                // Begin rendering
                if (application.task_processed) {
                    
                } else {
                    
                }
                
                LARGE_INTEGER work_counter = win32_get_wallclock();
                real32 work_seconds_elapsed = win32_get_seconds_elapsed(last_counter, work_counter);
                
                real32 seconds_elapsed_for_frame = work_seconds_elapsed;
                if (seconds_elapsed_for_frame < target_seconds_per_frame) {
                    DWORD sleep_ms = (DWORD)(1000.0f * (target_seconds_per_frame -
                                                        seconds_elapsed_for_frame));
                    if (sleep_ms > 0) {
                        Sleep(sleep_ms);
                    }
                    while (seconds_elapsed_for_frame < target_seconds_per_frame) {
                        seconds_elapsed_for_frame= win32_get_seconds_elapsed(last_counter,
                                                                             win32_get_wallclock());
                    }
                }
                
                // Get the frame time
                LARGE_INTEGER end_counter = win32_get_wallclock();
                real32 seconds_elapsed = win32_get_seconds_elapsed(last_counter, end_counter);
                real32 ms_per_frame = 1000.0f * seconds_elapsed;
                LONGLONG fps = (LONGLONG) frequency_counter / (end_counter.QuadPart - last_counter.QuadPart);
                
                if (timer_increment(change_clear_color_interval, ms_per_frame)) {
                    switch_colors(&clear_color);
                }
                
                application.current_time += ms_per_frame;
                
                last_counter = end_counter;
                
                char timing_buffer[256];
                _snprintf_s(timing_buffer, sizeof(timing_buffer),
                            "%.02f ms,  %lld fps", ms_per_frame, fps);
                v4 debug_color = make_color(1.f, 1.0f, 0.f);
                my_stbtt_print(&debug_font, margin,
                               application.height * 0.05f,
                               (u8*) timing_buffer, debug_color);
                SwapBuffers(hdc);
            }
            
            ReleaseDC(window, hdc);
        }
    }
    
    return 0;
}