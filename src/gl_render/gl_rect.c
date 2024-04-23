/*
 * gl_rect.c
 *
 * Setup and render a rectangle described by a 
 * GeometryRect in a GL context.
 *
 */

#include "chroma-engine.h"
#include "gl_render_internal.h"
#include "geometry.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

/*
 * Rectangle vertex layout (corners 
 * filled in by rounding)
 *
 *      10 --- 11 
 *   6 - 7     8 - 9  
 *   |             |
 *   2 - 3     4 - 5 
 *       0 --- 1  
 */

static unsigned int indices[] = {
    // bottom
    0, 1, 3,
    1, 4, 3,

    // left 
    2, 3, 6,
    3, 7, 6,

    // center 
    3, 4, 7,
    4, 8, 7,
    
    // right
    4, 5, 8,
    5, 9, 8,

    // top
    7, 8, 10,
    8, 11, 10,
};

static GeometryCircle *circle = NULL;

void gl_rectangle_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    // bind the vertex array object
    glBindVertexArray(vao);

    // bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( float ) * 12 * 3, NULL, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    // configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);

    circle = (GeometryCircle *)geometry_create_geometry(CIRCLE);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void gl_rectangle_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(INSTALL_DIR SHADER_PATH "glshape-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(INSTALL_DIR SHADER_PATH "glshape-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    free(vertexSource);
    free(fragmentSource);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gl_draw_rectangle(IGeometry *rect) {
    GeometryRect *g_rect = (GeometryRect *)rect;
    int pos_x = geometry_get_int_attr(rect, "pos_x");
    int pos_y = geometry_get_int_attr(rect, "pos_y");

    int width = g_rect->width;
    int height= g_rect->height;
    int round = MIN(g_rect->rounding, MIN(width / 2, height / 2));
    GLfloat vertices[] = {
        pos_x + round,         pos_y,                  0.0f,
        pos_x + width - round, pos_y,                  0.0f,

        pos_x,                 pos_y + round,          0.0f,
        pos_x + round,         pos_y + round,          0.0f,
        pos_x + width - round, pos_y + round,          0.0f,
        pos_x + width,         pos_y + round,          0.0f,

        pos_x,                 pos_y + height - round, 0.0f,
        pos_x + round,         pos_y + height - round, 0.0f,
        pos_x + width - round, pos_y + height - round, 0.0f,
        pos_x + width,         pos_y + height - round, 0.0f,

        pos_x + round,         pos_y + height,         0.0f,
        pos_x + width - round, pos_y + height,         0.0f,
    };

    int circ_v[] = {
        pos_x + round,         pos_y + round, 
        pos_x + width - round, pos_y + round,         

        pos_x + round,         pos_y + height - round,
        pos_x + width - round, pos_y + height - round,
    };

    // draw corners
    circle->color[0]     = g_rect->color[0];
    circle->color[1]     = g_rect->color[1];
    circle->color[2]     = g_rect->color[2];
    circle->color[3]     = g_rect->color[3];
    circle->inner_radius = 0;
    circle->outer_radius = round;

    for (int i = 0; i < 4; i++) {
        geometry_set_int_attr((IGeometry *)circle, "pos_x", circ_v[2 * i]);
        geometry_set_int_attr((IGeometry *)circle, "pos_y", circ_v[2 * i + 1]);
        gl_draw_circle((IGeometry *)circle);
    }

    gl_renderer_set_scale(program);
    //log_to_file(LogMessage, "Render rectangle %d %d %d %d", rect->pos_x, rect->pos_y, rect->width, rect->height)

    glUseProgram(program);
    glBindVertexArray(vao);

    unsigned int color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, g_rect->color[0], g_rect->color[1], 
                g_rect->color[2], g_rect->color[3]); 
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawElements(GL_TRIANGLES, 3 * 10, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
