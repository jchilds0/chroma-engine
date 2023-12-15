/*
 * Render an annulus using OpenGL
 */


#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "gl_renderer.h"

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

void gl_annulus_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    // bind the vertex array object
    glBindVertexArray(vao);

    // configure vertex attributes
    glBindVertexArray(0);
}

void gl_annulus_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// Find the number of triangles for the circle
static int gl_annulus_tri_num(int radius) {
    float n = 10; 
    float theta, A, B, E;

    do {
        theta = 2.0f * M_PI / n;
        A = theta * radius;
        B = 2 * radius * sinf(theta / 2);
        E = fabsf(A - B);
        n = n + 10;
    } while (E > 0.1f);

    return n;
} 

void gl_annulus_render(ChromaAnnulus *annulus) {
    int n = gl_annulus_tri_num(annulus->outer_radius);
    float theta = 2.0f * M_PI / n;

    GLfloat *vertices = NEW_ARRAY(6 * (n + 1), GLfloat);
    unsigned int *indices = NEW_ARRAY(6 * n, unsigned int);

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
        // inner radius point
        vertices[6 * i]     = annulus->inner_radius * cosf(theta * i) + annulus->center_x;
        vertices[6 * i + 1] = annulus->inner_radius * sinf(theta * i) + annulus->center_x;
        vertices[6 * i + 2] = 0.0f;

        // outer radius point
        vertices[6 * i + 3] = annulus->outer_radius * cosf(theta * i) + annulus->center_x;
        vertices[6 * i + 4] = annulus->outer_radius * sinf(theta * i) + annulus->center_x;
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
    //log_to_file(LogMessage, "Render rectangle %d %d %d %d", rect->pos_x, rect->pos_y, rect->width, rect->height)

    glUseProgram(program);
    glBindVertexArray(vao);

    uint color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, annulus->color[0], annulus->color[1], annulus->color[2], annulus->color[3]);
    
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

    free(vertices);
    free(indices);
}

