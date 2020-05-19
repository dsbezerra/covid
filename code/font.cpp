#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define BITMAP_SIZE 512

struct font {
    GLuint texture;
    stbtt_bakedchar cdata[224];
    stbtt_fontinfo info;
    float scale;
};

static font
my_stbtt_initfont(char *fontpath, float size) {
    font result = {};
    
    int buffer_size = 1 << 20;
    unsigned char *buffer = (unsigned char*) malloc(buffer_size);
    unsigned char *bitmap = (unsigned char*) malloc(BITMAP_SIZE * BITMAP_SIZE);
    
    FILE *file = fopen(fontpath, "rb");
    fread(buffer, 1, buffer_size, file);
    fclose(file);
    
    stbtt_InitFont(&result.info, buffer, 0);
    result.scale = stbtt_ScaleForPixelHeight(&result.info, size);
    stbtt_BakeFontBitmap(buffer, 0, size, bitmap, BITMAP_SIZE, BITMAP_SIZE, 32, 224, result.cdata);
    
    glGenTextures(1, &result.texture);
    glBindTexture(GL_TEXTURE_2D, result.texture);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, BITMAP_SIZE, BITMAP_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
    
    free(buffer);
    free(bitmap);
    
    return result;
}

static void
my_stbtt_print(font *font, float x, float y, unsigned char *text, v4 color) {
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
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