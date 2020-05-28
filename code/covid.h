#include "covid_types.h" 
#include "covid_math.h"

#include "font.cpp"
#include "myhtml_utils.cpp"

#define APP_WIDTH 500
#define APP_HEIGHT 500

enum state_type {
    State_NoData,
    State_OpeningConnection,
    State_ReceivingResponse,
    State_ResponseReceived,
    State_ReadingPage,
    State_ParsingPage,
    State_NetworkError,
    State_Processed,
    State_Count,
};

enum card_kind {
    Card_None,
    Card_Total,
    Card_Investigation,
    Card_Confirmed,
    Card_Discarded,
};

struct card {
    card_kind  kind;
    v4         background_color;
    u8         *label;
    u32        value;
    // Drawing properties
    real32     height;
    real32     center_y;
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

struct server_response {
    char *body;
    int content_length;
};

struct timer_interval {
    real32 current_ms;
    real32 interval_ms;
};

struct app {
    int width;
    int height;
    int current_state;
    
    real32 current_time;
    
    int fps;
    real32 dt;
    
    int fps_to_draw;
    real32 ms_per_frame_to_draw;
    
    font font_debug;
    font font_label;
    font font_value;
    
    timer_interval change_clear_color_interval;
    timer_interval frame_time_render_rate;
    
    bool show_debug_info;
    
    bool running;
    card cards[4];
    u8 *loaded_page;
};

struct dimension {
    int width;
    int height;
};

inline v4
make_color(real32 r, real32 g, real32 b) {
    return make_v4(r, g, b, 1.0f);
}

inline v4
make_color(real32 r, real32 g, real32 b, real32 a) {
    return make_v4(r, g, b, a);
}

inline v4 
make_color(u32 color) {
    real32 a = (0xff & (color >> 24)) / 255.f;
    real32 r = (0xff & (color >> 16)) / 255.f;
    real32 g = (0xff & (color >>  8)) / 255.f;
    real32 b = (0xff & (color >>  0)) / 255.f;
    if (a == 0.f) {
        a = 255.f;
    }
    return make_color(r, g, b, a);
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
#include "covid_opengl.cpp"
#include "covid_shader.cpp"
#include "covid_draw.cpp"

// 
// App
//
char * app_get_state_description(app *);

//
// Cards
//
card * get_card(app *, int);
card parse_card(myhtml_tree_node_t *node);
void set_card(app *application, card new_card);
