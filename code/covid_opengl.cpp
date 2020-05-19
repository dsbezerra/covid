// 
// GL Definitions
//

typedef char GLchar;

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

typedef void gl_get_shader_info_log(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void gl_get_shader_iv(GLuint shader, GLenum pname, GLint *params);
typedef void gl_get_program_iv(GLuint program, GLenum pname, GLint *params);
typedef void gl_get_program_info_log(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void gl_shader_source(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef GLuint gl_create_shader(GLenum type);
typedef void gl_compile_shader(GLuint shader);
typedef void gl_delete_shader(GLuint shader);
typedef int gl_create_program();
typedef void gl_attach_shader(GLuint program, GLuint shader);
typedef GLuint gl_link_program(GLuint program);
typedef void gl_use_program(GLuint program);
typedef void gl_gen_buffers(GLsizei n, GLuint * buffers);
typedef void gl_bind_vertex_array(GLuint array);
typedef void gl_bind_buffer(GLenum target, GLuint buffer);
typedef void gl_buffer_data(GLenum target, GLsizei size, const void * data, GLenum usage);
typedef void gl_vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
typedef void gl_enable_vertext_attrib_array(GLuint index);
typedef void gl_gen_vertex_arrays(GLsizei n, GLuint *arrays);

typedef GLint gl_get_uniform_location(GLuint program, const GLchar *name);
typedef void gl_uniform_4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

//
// Currently used functions
//

gl_get_shader_info_log *glGetShaderInfoLog;
gl_get_shader_iv *glGetShaderiv;
gl_get_program_iv *glGetProgramiv;
gl_get_program_info_log *glGetProgramInfoLog;
gl_shader_source *glShaderSource;
gl_create_shader *glCreateShader;
gl_compile_shader *glCompileShader;
gl_delete_shader *glDeleteShader;
gl_create_program *glCreateProgram;
gl_attach_shader *glAttachShader;
gl_link_program *glLinkProgram;
gl_use_program *glUseProgram;
gl_gen_buffers *glGenBuffers;
gl_bind_vertex_array *glBindVertexArray;
gl_bind_buffer *glBindBuffer;
gl_buffer_data *glBufferData;
gl_vertex_attrib_pointer *glVertexAttribPointer;
gl_enable_vertext_attrib_array *glEnableVertexAttribArray;
gl_gen_vertex_arrays *glGenVertexArrays;

gl_uniform_4f *glUniform4f;
gl_get_uniform_location *glGetUniformLocation;;

void win32_opengl_get_functions() {
    glGetShaderInfoLog   = (gl_get_shader_info_log *)  wglGetProcAddress("glGetShaderInfoLog");
    glGetShaderiv        = (gl_get_shader_iv *)        wglGetProcAddress("glGetShaderiv");
    
    glGetProgramiv       = (gl_get_program_iv *)       wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog  = (gl_get_program_info_log *) wglGetProcAddress("glGetProgramInfoLog");
    
    glShaderSource   = (gl_shader_source *)  wglGetProcAddress("glShaderSource");
    glCreateShader   = (gl_create_shader *)  wglGetProcAddress("glCreateShader");
    glCompileShader  = (gl_compile_shader *) wglGetProcAddress("glCompileShader");
    glDeleteShader   = (gl_delete_shader *)  wglGetProcAddress("glDeleteShader");
    
    glCreateProgram  = (gl_create_program *) wglGetProcAddress("glCreateProgram");
    glAttachShader   = (gl_attach_shader *)  wglGetProcAddress("glAttachShader");
    glLinkProgram    = (gl_link_program *)   wglGetProcAddress("glLinkProgram");
    glUseProgram     = (gl_use_program *)    wglGetProcAddress("glUseProgram");
    
    glGenBuffers = (gl_gen_buffers *) wglGetProcAddress("glGenBuffers");
    glBindVertexArray = (gl_bind_vertex_array *) wglGetProcAddress("glBindVertexArray");
    glBindBuffer = (gl_bind_buffer *) wglGetProcAddress("glBindBuffer");
    glBufferData = (gl_buffer_data *) wglGetProcAddress("glBufferData");
    glVertexAttribPointer = (gl_vertex_attrib_pointer *) wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (gl_enable_vertext_attrib_array *) wglGetProcAddress("glEnableVertexAttribArray");
    glGenVertexArrays = (gl_gen_vertex_arrays *) wglGetProcAddress("glGenVertexArrays");
    
    glGetUniformLocation = (gl_get_uniform_location *) wglGetProcAddress("glGetUniformLocation");
    glUniform4f = (gl_uniform_4f *) wglGetProcAddress("glUniform4f");
}

//
// Some functions
//

void
resize_viewport(int width, int height) {
    // Set the "drawable region"
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, -1);
    
    float a = 2.0f / width;
    float b = 2.0f / -height;
    float proj[] = {
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, 1, 0,
        -1, 1, 0, 1,
    };
    glLoadMatrixf(proj);
}
