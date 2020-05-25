#include "covid_types.h"
#include "covid_math.h"

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

struct server_response {
    char *body;
    int content_length;
};

struct app {
    int width;
    int height;
    int current_state;
    
    real32 current_time;
    
    int fps;
    real32 dt;
    
    bool running;
    bool show_debug_info;
    card cards[4];
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