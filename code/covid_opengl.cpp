#include "covid_opengl.h"

// 
// GL Definitions
//

#define GL_FRAGMENT_SHADER      0x8B30
#define GL_VERTEX_SHADER        0x8B31
#define GL_COMPILE_STATUS       0x8B81
#define GL_LINK_STATUS          0x8B82
#define GL_INFO_LOG_LENGTH      0x8B84

// 
// Buffer Objects
//

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

//
// Texture
//

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1

opengl_info  
opengl_get_info() {
    opengl_info result = {};
    result.vendor = (char *) glGetString(GL_VENDOR);
    result.renderer = (char *) glGetString(GL_RENDERER);
    result.version = (char *) glGetString(GL_VERSION);
    return result;
}
