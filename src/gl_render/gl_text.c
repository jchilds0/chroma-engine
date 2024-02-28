/*
 * gl_text.c
 *
 * Setup and render text described by a 
 * GeometryText in a GL context.
 *
 */

#include "chroma-engine.h"
#include "gl_render_internal.h"
#include "geometry.h"
#include "log.h"

#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static GLuint vao;
static GLuint vbo;
static GLuint program;

struct Character {
    unsigned int TextureID;
    GLfloat      Size[2];
    GLfloat      Bearing[2];
    unsigned int Advance;
};

static struct Character Characters[128];

void gl_text_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( float ) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
} 

void gl_text_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(INSTALL_DIR SHADER_PATH "gltext-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(INSTALL_DIR SHADER_PATH "gltext-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    free(vertexSource);
    free(fragmentSource);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gl_text_cache_characters(void) {
    // init characters
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        log_file(LogError, "Geometry", "Couldn't init FreeType Library");
    }

    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf", 0, &face)) {
        log_file(LogError, "Geometry", "Failed to load font");
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            log_file(LogWarn, "Geometry", "Failed to load Glyph %c", c);
        }

        // generate texture 
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // store character 
        Characters[c] = (struct Character){
            texture,
            {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            {face->glyph->bitmap_left, face->glyph->bitmap_top},
            face->glyph->advance.x
        };
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void gl_draw_text(IGeometry *text) {
    int text_x = geometry_get_int_attr(text, "pos_x");
    int text_y = geometry_get_int_attr(text, "pos_y");

    char buf[100];
    GLfloat r, g, b, a, scale;

    memset(buf, '\0', sizeof buf);
    geometry_get_attr(text, "color", buf);
    sscanf(buf, "%f %f %f %f", &r, &g, &b, &a);

    memset(buf, '\0', sizeof buf);
    geometry_get_attr(text, "scale", buf);
    sscanf(buf, "%f", &scale);

    memset(buf, '\0', sizeof buf);
    geometry_get_attr(text, "string", buf);

    // Copy vertices array in a buffer for OpenGL
    gl_renderer_set_scale(program);

    glUseProgram(program);

    GLint color_loc = glGetUniformLocation(program, "color");
    glUniform3f(color_loc, r, g, b);
    glBindVertexArray(vao);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; buf[i] != '\0'; i++) {
        struct Character ch = Characters[(unsigned char)buf[i]];

        float xpos = text_x + ch.Bearing[0] * scale;
        float ypos = text_y - (ch.Size[1] - ch.Bearing[1]) * scale;

        float w = ch.Size[0] * scale;
        float h = ch.Size[1] * scale;

        // update vbo
        float vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos,     ypos,     0.0f, 1.0f },
            { xpos + w, ypos,     1.0f, 1.0f },

            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f },
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // update content of vbo memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance cursors for next glyph
        text_x += (ch.Advance >> 6) * scale;
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

