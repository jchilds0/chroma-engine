/*
 * gl_poly.c
 *
 * Setup and render a rectangle described by a 
 * GeometryPolygon in a GL context.
 *
 */

#include "chroma-engine.h"
#include "geometry/geometry_internal.h"
#include "gl_render_internal.h"
#include "geometry.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdlib.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

static GLfloat vertices[(MAX_NODES + 1) * 3];
static unsigned int indices[MAX_NODES * 3];

void gl_polygon_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
}

void gl_polygon_init_shaders(void) {
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

void gl_draw_polygon(IGeometry *poly) {
    int pos_x = geometry_get_int_attr(poly, GEO_POS_X);
    int pos_y = geometry_get_int_attr(poly, GEO_POS_Y); 
    int num_points = geometry_get_int_attr(poly, GEO_NUM_POINTS);

    if (num_points == 0) {
        return;
    }

    GeometryPolygon *g_poly = (GeometryPolygon *)poly;

    int center_x = 0, center_y = 0;
    vec2 vec;
    for (int i = 0; i < num_points; i++) {
        vec = geometry_polygon_get_point(g_poly, i);

        vertices[3 * i] = vec.x + pos_x;
        vertices[3 * i + 1] = vec.y + pos_y;
        vertices[3 * i + 2] = 0;

        indices[3 * i] = i;
        indices[3 * i + 1] = (i + 1) % num_points;
        indices[3 * i + 2] = num_points;

        center_x += vec.x;
        center_y += vec.y;
    }

    vertices[3 * num_points] = pos_x + (center_x / num_points);
    vertices[3 * num_points + 1] = pos_y + (center_y / num_points);
    vertices[3 * num_points + 2] = 0;

    gl_renderer_set_scale(program);

    glUseProgram(program);
    glBindVertexArray(vao);

    unsigned int color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, g_poly->color[0], g_poly->color[1], 
                g_poly->color[2], g_poly->color[3]); 
    
    // bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( GLfloat ) * 3 * (num_points + 1), vertices, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int ) * 3 * num_points, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, 3 * num_points, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
