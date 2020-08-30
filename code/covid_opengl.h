/* date = May 19th 2020 9:29 pm */

struct opengl_info {
    char *vendor;
    char *renderer;
    char *version;
};

typedef BOOL type_wglSwapIntervalEXT(int interval);

#define opengl_function(name) type_##name *name
#define opengl_get_function(name) open_gl->name = (type_##name *) wglGetProcAddress(#name)

struct opengl {
    opengl_info info;
    opengl_function(wglSwapIntervalEXT);
};

opengl *open_gl;