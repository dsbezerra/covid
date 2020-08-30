opengl_info  
opengl_get_info() {
    opengl_info result = {};
    result.vendor = (char *) glGetString(GL_VENDOR);
    result.renderer = (char *) glGetString(GL_RENDERER);
    result.version = (char *) glGetString(GL_VERSION);
    return result;
}
