u32 quad_shader;
u32 quad_ebo;
u32 quad_vbo;
u32 quad_vao;

void
init_quad_objects() {
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glGenBuffers(1, &quad_ebo);
    
    quad_shader = init_shaders();
}

void
draw_quad(float x0, float y0, float x1, float y1, v4 color) {
    float vertices[] = {
        x1, y1, 0.f, // top right  
        x1, y0, 0.f, // bottom right 
        x0, y0, 0.f, // bottom left
        x0, y1, 0.f, // top left
    }; 
    
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    
    set_shader(quad_shader);
    set_uniform_v4(quad_shader, "colour", color);
    
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (float), 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void
draw_text(real32 x, real32 y, u8 *text, font *font, v4 color) {
    // @Incomplete reset shader so we can render our text for now.
    set_shader(0);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glBegin(GL_QUADS);
    glColor4f(color.r, color.g, color.b, color.a);
    
    while (*text) {
        if (*text >= 32 && *text < 255) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->cdata, BITMAP_SIZE, BITMAP_SIZE, *text - 32, &x, &y, &q, 1);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0,q.y0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0,q.y1);
        }
        ++text;
    }
    glEnd();
    glDisable(GL_BLEND);
}
