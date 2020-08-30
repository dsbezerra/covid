global_variable shader global_shader;

internal void
check_compile_error(int shader) {
    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, 0, log);
        OutputDebugString("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        OutputDebugString(log);
        OutputDebugString("\n");
    }
}

internal void
check_linking_error(int program) {
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLsizei ignored;
        char program_errors[4096];
        glGetProgramInfoLog(program, sizeof(program_errors), &ignored, program_errors);
        
        OutputDebugString("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
        if (program_errors[0] != 0) {
            OutputDebugString(program_errors);
            OutputDebugString("\n");
        }
        OutputDebugString("\n");
    }
}

internal int
compile_shader(int type, char *source) {
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    check_compile_error(shader);
    return shader;
}

internal int
link_shaders(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int program = glCreateProgram();
    int *shaders = new int[n];
    int *at = shaders;
    for (int i = 0; i < n; i++) {
        int shader = va_arg(ap, int);
        *at++ = shader;
        glAttachShader(program, shader);
    }
    va_end(ap);
    glLinkProgram(program);
    check_linking_error(program);
    
    while (*at) {
        glDetachShader(program, *at);
        *at++;
    }
    
    return program;
}

internal void
delete_shader(int shader) {
    glDeleteShader(shader);
}

internal void
delete_shaders(int n, ...) {
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; i++) {
        int shader = va_arg(ap, int);
        delete_shader(shader);
    }
    va_end(ap);
}

internal void
init_shaders() {
    shader result = {};
    char *vertex = "#version 330 core\n#extension GL_ARB_separate_shader_objects: enable\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in vec4 color;\n"
        "layout (location = 2) in vec2 uv;\n"
        "layout (location = 3) in float z_index;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "out vec4 out_color;\n"
        "out vec2 out_uv;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * vec4(position, -z_index, 1.0);\n"
        "   out_color = color;\n"
        "   out_uv = uv;\n"
        "}\n\0";
    char *fragment = "#version 330 core\n#extension GL_ARB_separate_shader_objects: enable\n"
        "#extension GL_ARB_separate_shader_objects: enable\n"
        "out vec4 frag_color;\n"
        "in vec4 out_color;\n"
        "in vec2 out_uv;\n"
        "uniform sampler2D ftex;\n"
        "void main()\n"
        "{\n"
        "   if (out_uv.x < 0 && out_uv.y < 0) frag_color = out_color;\n"
        "   else {\n"
        "     vec4 sample = texture(ftex, out_uv);\n"
        "     frag_color = vec4(out_color.xyz, sample.a);\n"
        "   }\n"
        "}\n\0";
    int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex);
    int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment);
    int program = link_shaders(2, vertex_shader, fragment_shader);
    delete_shaders(2, vertex_shader, fragment_shader);
    
    result.program = program;
    result.projection_loc = glGetUniformLocation(program, "projection");
    result.view_loc = glGetUniformLocation(program, "view");
    result.texture_loc = glGetUniformLocation(program, "ftex");
    result.position_loc = 0;
    result.color_loc    = 1;
    result.uv_loc       = 2;
    result.z_index_loc  = 3;
    
    global_shader = result;
}
