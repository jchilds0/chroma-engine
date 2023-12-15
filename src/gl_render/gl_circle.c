/*
 * Render a circle using OpenGL
 */

#include "chroma-engine.h"
#include "gl_renderer.h"

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

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
    char *vertexSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// Find the number of triangles for the circle
static int gl_circle_tri_num(int radius) {
    float n = 10; 
    float theta, A, B, E;

    do {
        theta = 2.0f * M_PI / n;
        A = theta * radius;
        B = 2 * radius * sinf(theta / 2);
        E = fabsf(A - B);
        n = n + 10;
    } while (E > 0.001f);

    return n;
} 

void gl_circle_render(ChromaCircle *circle) {
    int n = gl_circle_tri_num(circle->radius);
    float theta = 2.0f * M_PI / n;

    GLfloat *vertices = NEW_ARRAY(3 * (n + 2), GLfloat);
    unsigned int *indices = NEW_ARRAY(3 * (n + 1), unsigned int);
    vertices[0] = circle->center_x;
    vertices[1] = circle->center_y;
    vertices[2] = 0.0f;

    for (int i = 0; i < n + 1; i++) {
        vertices[3 * (i + 1)]     = circle->radius * cosf(theta * i) + circle->center_x;
        vertices[3 * (i + 1) + 1] = circle->radius * sinf(theta * i) + circle->center_y;
        vertices[3 * (i + 1) + 2] = 0.0f; 
    }

    for (int i = 0; i < n + 1; i++) {
        indices[3 * i]     = i + 1;
        indices[3 * i + 1] = 0;
        indices[3 * i + 2] = i + 2;
    }

    gl_renderer_set_scale(program);
    //log_to_file(LogMessage, "Render rectangle %d %d %d %d", rect->pos_x, rect->pos_y, rect->width, rect->height)

    glUseProgram(program);
    glBindVertexArray(vao);

    uint color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, circle->color[0], circle->color[1], circle->color[2], circle->color[3]);
    
    // bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( GLfloat ) * 3 * (n + 2), vertices, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int ) * 3 * (n + 1), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, 3 * n, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    free(vertices);
    free(indices);
}

