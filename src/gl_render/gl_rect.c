/*
 *
 */

#include "gl_render_internal.h"
#include "geometry.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

static unsigned int indices[] = {
    0, 1, 3, // first triangle 
    1, 2, 3  // second triangle
};

void gl_rectangle_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    // bind the vertex array object
    glBindVertexArray(vao);

    // bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( float ) * 4 * 3, NULL, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    // configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void gl_rectangle_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gl_draw_rectangle(IGeometry *rect) {
    int pos_x = geometry_get_int_attr(rect, "pos_x");
    int pos_y = geometry_get_int_attr(rect, "pos_y");
    int width = geometry_get_int_attr(rect, "width");
    int height= geometry_get_int_attr(rect, "height");

    char buf[100];
    GLfloat r, g, b, a;

    memset(buf, '\0', sizeof buf);
    geometry_get_attr(rect, "color", buf);
    sscanf(buf, "%f %f %f %f", &r, &g, &b, &a);

    GLfloat vertices[] = {
        pos_x,         pos_y + height, 0.0f,
        pos_x,         pos_y,          0.0f,
        pos_x + width, pos_y,          0.0f,
        pos_x + width, pos_y + height, 0.0f,
    };

    gl_renderer_set_scale(program);
    //log_to_file(LogMessage, "Render rectangle %d %d %d %d", rect->pos_x, rect->pos_y, rect->width, rect->height)

    glUseProgram(program);
    glBindVertexArray(vao);

    unsigned int color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, r, g, b, a); 
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
