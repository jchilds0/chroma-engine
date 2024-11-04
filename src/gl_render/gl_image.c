/*
 * gl_image.c
 *
 * Setup and render an image described by a 
 * GeometryImage in a GL context.
 *
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "gl_render_internal.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint texture;
static GLuint program;

static GLuint indices[] = {
    0, 1, 3, // first triangle 
    1, 2, 3, // second triangle
};

void gl_image_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // bind buffer arrays 
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( float ) * 4 * 8, NULL, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    // position attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)(0));
    glEnableVertexAttribArray(0);

    // color attribute 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)(3 * sizeof( float )));
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void *)(6 * sizeof( float )));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void gl_image_init_shaders(void) {
    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, glimage_vs_glsl);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, glimage_fs_glsl);

    program = gl_renderer_create_program(vertex, fragment);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gl_draw_image(IGeometry *geo) {
    GeometryImage *img = (GeometryImage *)geo;
    int pos_x = geometry_get_int_attr(geo, GEO_POS_X);
    int pos_y = geometry_get_int_attr(geo, GEO_POS_Y);

    if (img->data == NULL) {
        return;
    }

    char buf[100];
    memset(buf, '\0', sizeof( buf ));
    geometry_get_attr(geo, "scale", buf);
    float scale = atof(buf);

    GLfloat vertices[] = {
        // positions                                            // colors           // texture coords
        pos_x + img->w * scale, pos_y + img->h * scale, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        pos_x + img->w * scale, pos_y,                  0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        pos_x,                  pos_y,                  0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom right
        pos_x,                  pos_y + img->h * scale, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f, // bottom right
    };

    gl_renderer_set_scale(program);

    glUseProgram(program);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
