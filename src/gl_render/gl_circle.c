/*
 * gl_circle.c
 *
 * Setup and render an circle described by a 
 * GeometryAnnulus in a GL context.
 *
 */

#include "chroma-engine.h"
#include "gl_render_internal.h"
#include "geometry.h"
#include <GL/gl.h>
#include <stdlib.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

static int num_nodes         = -1;
static GLfloat *vertices     = NULL;
static unsigned int *indices = NULL;

void gl_circle_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    // bind the vertex array object
    glBindVertexArray(vao);

    // configure vertex attributes
    glBindVertexArray(0);
}

void gl_circle_init_shaders(void) {
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

// Find the number of triangles for the circle
static int gl_circle_tri_num(int radius, float start_angle, float end_angle) {
    float n = 10; 
    float theta, A, B, E;

    do {
        theta = (end_angle - start_angle) / n;
        A = theta * radius;
        B = 2 * radius * sinf(theta / 2);
        E = fabsf(A - B);
        n = 1.5 * n;
    } while (E > 0.01f && n < 1000);

    return n;
} 

void gl_draw_circle(IGeometry *circle) {
    int center_x = geometry_get_int_attr(circle, GEO_POS_X);
    int center_y = geometry_get_int_attr(circle, GEO_POS_Y);

    int inner_radius = geometry_get_int_attr(circle, GEO_INNER_RADIUS);
    int outer_radius = geometry_get_int_attr(circle, GEO_OUTER_RADIUS);
    float start_angle = geometry_get_int_attr(circle, GEO_START_ANGLE) * M_PI / 180;
    float end_angle = geometry_get_int_attr(circle, GEO_END_ANGLE) * M_PI / 180;

    float cos_theta, sin_theta;

    int n = gl_circle_tri_num(outer_radius, start_angle, end_angle);
    if (num_nodes != n) {
        free(vertices);
        free(indices);

        num_nodes = n;
        vertices  = NEW_ARRAY(6 * (n + 1), GLfloat);
        indices   = NEW_ARRAY(6 * n, unsigned int);
    }

    float theta = (float)(end_angle - start_angle) / n;

    /*
     * Create a quadrilateral out of the 4 points 
     *
     *    inner_radius * e^(i * theta * index)
     *    outer_radius * e^(i * theta * index)
     *    inner_radius * e^(i * theta * (index + 1))
     *    outer_radius * e^(i * theta * (index + 1))
     * 
     * using two triangles
     */

    for (int i = 0; i < n + 1; i++) {
        cos_theta = cosf(theta * i + start_angle);
        sin_theta = sinf(theta * i + start_angle);

        // inner radius point
        vertices[6 * i]     = inner_radius * cos_theta + center_x;
        vertices[6 * i + 1] = inner_radius * sin_theta + center_y;
        vertices[6 * i + 2] = 0.0f;

        // outer radius point
        vertices[6 * i + 3] = outer_radius * cos_theta + center_x;
        vertices[6 * i + 4] = outer_radius * sin_theta + center_y;
        vertices[6 * i + 5] = 0.0f;
    }

    for (int i = 0; i < n; i++) {
        indices[6 * i]     = 2 * i;
        indices[6 * i + 1] = 2 * i + 1;
        indices[6 * i + 2] = 2 * i + 3;

        indices[6 * i + 3] = 2 * i + 2;
        indices[6 * i + 4] = 2 * i;
        indices[6 * i + 5] = 2 * i + 3;
    }

    gl_renderer_set_scale(program);

    glUseProgram(program);
    glBindVertexArray(vao);

    uint color_loc = glGetUniformLocation(program, "color");
    GeometryCircle *g_circle = (GeometryCircle *)circle;
    glUniform4f(color_loc, g_circle->color[0], g_circle->color[1], 
                g_circle->color[2], g_circle->color[3]);
    
    // bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( GLfloat ) * 6 * (n + 1), vertices, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int ) * 6 * n, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, 6 * n, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

