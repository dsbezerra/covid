
static void
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

static void
check_linking_error(int program) {
    int success;
    char log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, 0, log);
        OutputDebugString("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
        OutputDebugString(log);
        OutputDebugString("\n");
    }
}

static int
compile_shader(int type, char *source) {
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    check_compile_error(shader);
    return shader;
}

static int
link_shaders(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int program = glCreateProgram();
    for (int i = 0; i < n; i++) {
        int shader = va_arg(ap, int);
        glAttachShader(program, shader);
    }
    va_end(ap);
    glLinkProgram(program);
    check_linking_error(program);
    return program;
}

static void
delete_shader(int shader) {
    glDeleteShader(shader);
}

static void
delete_shaders(int n, ...) {
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; i++) {
        int shader = va_arg(ap, int);
        delete_shader(shader);
    }
    va_end(ap);
}

unsigned int
init_shaders() {
    char *vertex = "#version 330 core\n"
        "layout (location = 0) in vec3 position;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
        "}\0";
    char *fragment = "#version 330 core\n"
        "out vec4 color;\n"
        "uniform vec4 colour;\n"
        "void main()\n"
        "{\n"
        "   color = colour;\n"
        "}\n\0";
    int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex);
    int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment);
    int program = link_shaders(vertex_shader, fragment_shader);
    return program;
}

void
set_shader(int shader_program) {
    glUseProgram(shader_program);
}

void
set_uniform_v4(int shader_program, GLchar *name, v4 v) {
    glUniform4f(glGetUniformLocation(shader_program, name), v.x, v.y, v.z, v.w);
}