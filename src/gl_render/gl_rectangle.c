/*
 * Render a rectangle using OpenGL
 */

#include "chroma-prototypes.h"
#include "chroma-typedefs.h"
#include "gl_renderer.h"
#include <GL/gl.h>
#include <GL/glext.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

static unsigned int indices[] = {
    0, 1, 3, // first triangle 
    1, 2, 3  // second triangle
};

void gl_rect_init_buffers(void) {
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

void gl_rect_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(SHADER_PATH "glrect-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(SHADER_PATH "glrect-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gl_rect_render(Chroma_Rectangle *rect) {
    GLfloat vertices[] = {
        rect->pos_x,               rect->pos_y + rect->height, 0.0f,
        rect->pos_x,               rect->pos_y,                0.0f,
        rect->pos_x + rect->width, rect->pos_y,                0.0f,
        rect->pos_x + rect->width, rect->pos_y + rect->height, 0.0f,
    };

    gl_renderer_set_scale(program);
    //log_to_file(LogMessage, "Render rectangle %d %d %d %d", rect->pos_x, rect->pos_y, rect->width, rect->height);

    glUseProgram(program);
    glBindVertexArray(vao);

    uint color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, rect->color[0], rect->color[1], rect->color[2], rect->color[3]);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

