/* date = May 21st 2020 7:06 pm */

#define MAX_VERTICES 2400

struct vertex {
    v2 position;
    v4 color;
    v2 uv;
    real32 z_index;
};

struct imm {
    
    shader current_shader;
    
    GLuint vao;
    GLuint vbo;
    
    vertex vertices[MAX_VERTICES];
    
    int num_vertices;
};

internal void immediate_flush();
// TODO(diego): Add others