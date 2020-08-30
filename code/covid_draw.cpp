global_variable imm* immediate;
global_variable mat4 view_matrix;
global_variable mat4 projection_matrix;

internal void
immediate_init() {
    if (immediate) {
        return;
    }
    
    immediate = (imm *) malloc(sizeof(imm));
    
    glGenVertexArrays(1, &immediate->vao);
    glBindVertexArray(immediate->vao);
    
    glGenBuffers(1, &immediate->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, immediate->vbo);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

internal void
immediate_free() {
    free(immediate);
}

internal void
immediate_begin() {
    assert(immediate);
    
    immediate->num_vertices = 0;
}

internal vertex * 
get_next_vertex_ptr() {
	return (vertex *) &immediate->vertices + immediate->num_vertices;
}

internal void
immediate_vertex(v2 position, v4 color, v2 uv, real32 z_index) {
    
    if (immediate->num_vertices > MAX_VERTICES - 6) {
		immediate_flush();
		immediate_begin();
	}
    
    vertex *v = get_next_vertex_ptr();
    v->position.x = position.x;
    v->position.y = -position.y;
    v->color    = color;
    v->uv       = uv;
    v->z_index  = z_index;
    
    immediate->num_vertices += 1;
}

internal void
immediate_quad(float x0, float y0, float x1, float y1, v4 color, real32 z_index) {
    assert(immediate);
    
    if (immediate->num_vertices > MAX_VERTICES - 6) {
        immediate_flush();
    }
    
    int vcount = immediate->num_vertices;
    v2 default_uv = make_v2(-1.f, -1.f);
    
    immediate_vertex(make_v2(x0, y0), color, default_uv, z_index);
    immediate_vertex(make_v2(x0, y1), color, default_uv, z_index);
    immediate_vertex(make_v2(x1, y0), color, default_uv, z_index);
    
    immediate_vertex(make_v2(x0, y1), color, default_uv, z_index);
    immediate_vertex(make_v2(x1, y1), color, default_uv, z_index);
    immediate_vertex(make_v2(x1, y0), color, default_uv, z_index);
}

internal void
immediate_flush() {
    assert(immediate);
    
    int count = immediate->num_vertices;
    if (count == 0) {
        return;
    }
    
    if (!immediate->current_shader.program) {
        // TODO(diego): Log messages!
        immediate->num_vertices = 0;
        return;
    }
    
    glBindVertexArray(immediate->vao);
    glBindBuffer(GL_ARRAY_BUFFER, immediate->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(immediate->vertices[0]) * count, immediate->vertices, GL_STREAM_DRAW);
    
    GLint position_loc = immediate->current_shader.position_loc;
    GLint color_loc = immediate->current_shader.color_loc;
    GLint uv_loc = immediate->current_shader.uv_loc;
    GLint z_index_loc = immediate->current_shader.z_index_loc;
    
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(position_loc);
    
    glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) sizeof(v2));
    glEnableVertexAttribArray(color_loc);
    
    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v2) + sizeof(v4)));
    glEnableVertexAttribArray(uv_loc);
    
    glVertexAttribPointer(z_index_loc, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(v2) + sizeof(v4) + sizeof(v2)));
    glEnableVertexAttribArray(z_index_loc);
    
    glDrawArrays(GL_TRIANGLES, 0, immediate->num_vertices);
    
    glDisableVertexAttribArray(position_loc);
    glDisableVertexAttribArray(color_loc);
    glDisableVertexAttribArray(uv_loc);
    glDisableVertexAttribArray(z_index_loc);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    immediate->num_vertices = 0;
}

internal void
set_shader(shader new_shader) {
    assert(immediate);
    
    immediate->current_shader = new_shader;
    glUseProgram(new_shader.program);
}

internal void
refresh_shader_transform() {
    if (!immediate) {
        return;
    }
    
    if (!immediate->current_shader.program) {
        return;
    }
    
    glUniformMatrix4fv(immediate->current_shader.view_loc, 1, GL_FALSE, view_matrix.e);
    glUniformMatrix4fv(immediate->current_shader.projection_loc, 1, GL_FALSE, projection_matrix.e);
}

internal void
render_right_handed(int width, int height) {
    
    mat4 tm = identity();
    tm.rc[0][0] = 2.f / width;
    tm.rc[1][1] = 2.f / height;
    
    tm.rc[3][0] = -1.f;
    tm.rc[3][1] = 1.f;
    
    projection_matrix = tm;
    view_matrix       = identity();
    
    refresh_shader_transform();
}

internal void
draw_text(real32 x, real32 y, u8 *text, loaded_font *font, v4 color, real32 z_index) {
    
    immediate_begin();
    
    glUniform1i(immediate->current_shader.texture_loc, 0);
    
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glActiveTexture(GL_TEXTURE0);
    
    real32 start = x;
    
    while (*text) {
        if (*text >= 32 && *text < 255) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->cdata, BITMAP_SIZE, BITMAP_SIZE, *text - 32, &x, &y, &q, 1);
            
            v2 bottom_right = make_v2(q.s1, q.t0);
            v2 bottom_left  = make_v2(q.s0, q.t0);
            v2 top_right    = make_v2(q.s1, q.t1);
            v2 top_left     = make_v2(q.s0, q.t1);
            
            immediate_vertex(make_v2(q.x0, q.y0), color, bottom_left, z_index);
            immediate_vertex(make_v2(q.x0, q.y1), color, top_left, z_index);
            immediate_vertex(make_v2(q.x1, q.y0), color, bottom_right, z_index);
            
            immediate_vertex(make_v2(q.x0, q.y1), color, top_left, z_index);
            immediate_vertex(make_v2(q.x1, q.y1), color, top_right, z_index);
            immediate_vertex(make_v2(q.x1, q.y0), color, bottom_right, z_index);
            
        } else if (*text == '\n') {
            y += font->line_height;
            x = start;
        }
        ++text;
    }
    
    immediate_flush();
}

internal real32
get_text_width(loaded_font *my_font, char *text, int *line_count) {
    real32 result = 0.f;
    real32 w = 0.f;
    
    if (line_count) {
        *line_count = 0;
    }
    
    char *at = text;
    while (*at) {
        if (*at == '\n' && line_count) {
            if (w > result) {
                result = w;
            }
            w = .0f;
            (*line_count)++;
        } else if (*at >= 32 && *at < 255) {
            w += my_font->cdata[*at].xadvance;
        }
        *at++;
    }
    
    if (w > result) {
        result = w;
    }
    
    if (line_count) {
        (*line_count)++;
    } 
    
    return result;
}


internal real32
get_text_width(loaded_font *my_font, char *text) {
    return get_text_width(my_font, text, 0);
}

internal void
draw_debug(app *application, loaded_font *debug_font, real32 ms_per_frame, int fps) {
    char debug_buffer[256];
    v4 debug_color = make_color(1.f, 1.0f, 0.f);
    v4 debug_back_color = make_color(4 / 255.f, 122 / 255.f, 200 / 255.f, .9f);
    
    opengl_info info = open_gl->info;
    _snprintf_s(debug_buffer, sizeof(debug_buffer),
                "%s\n%s\n%s\n%.02f ms, %d fps", info.vendor, info.renderer, info.version, ms_per_frame, fps);
    
    int line_count;
    real32 box_width = get_text_width(debug_font, debug_buffer, &line_count);
    real32 box_height = line_count * debug_font->line_height;
    real32 box_pad = 6.f;
    
    real32 margin = 12.f;
    
    real32 box_x0 = application->width - box_width - margin - box_pad;
    real32 box_x1 = box_x0 + box_width + box_pad; 
    real32 box_y0 = margin;
    real32 box_y1 = box_y0 + box_height + box_pad;
    
    real32 text_y = box_y0 + debug_font->line_height;
    
    immediate_begin();
    immediate_quad(box_x0, box_y0, box_x1, box_y1, debug_back_color, 1.f);
    immediate_flush();
    
    draw_text(box_x0, text_y, (u8*) debug_buffer, debug_font, debug_color, 1.f);
}
