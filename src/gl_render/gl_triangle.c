/*
 * gl_triangle.c
 */

#include "gl_render_internal.h"

void gl_renderer_triangle_init(Renderer *r, const char *vs, const char *fs) {
    {
        glGenVertexArrays(1, &r->vao);
        glBindVertexArray(r->vao);

        glGenBuffers(1, &r->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof( r->triangles ), r->triangles, GL_DYNAMIC_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(VERTEX_ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), 
                              (GLvoid *)offsetof(Vertex, pos));

        // color 
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(VERTEX_ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), 
                              (GLvoid *)offsetof(Vertex, color));

        // texture
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(VERTEX_ATTR_UV, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), 
                              (GLvoid *)offsetof(Vertex, uv));
    }

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vs);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fs);

    r->program = gl_renderer_create_program(vertex, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gl_renderer_use(Renderer *r) {
    glUseProgram(r->program);
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    gl_renderer_set_scale(r->program);
}

void gl_renderer_triangle(Renderer *r, vec2 p0, vec2 p1, vec2 p2, 
                          vec4 c0, vec4 c1, vec4 c2, vec2 uv0, vec2 uv1, vec2 uv2) {

    log_assert(r->count < TRIANGLE_CAP, "GL Renderer", "Too many triangles");
    Triangle *tri = &r->triangles[r->count++];

    tri->v[0].pos = p0;
    tri->v[1].pos = p1;
    tri->v[2].pos = p2;

    tri->v[0].color = c0;
    tri->v[1].color = c1;
    tri->v[2].color = c2;

    tri->v[0].uv = uv0;
    tri->v[1].uv = uv1;
    tri->v[2].uv = uv2;
}

void gl_renderer_mask(Renderer *r, RendererOptions opt, int depth) {
    GLuint ones = (1 << depth) - 1;

    switch (opt) {
        case RENDER_DRAW_NO_MASK:
            glEnable(GL_BLEND);
            glStencilMask(0x00);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            break;

        case RENDER_DRAW_MASK:
            glEnable(GL_BLEND);
            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, ones, ones);
            break;

        case RENDER_MASK:
            glDisable(GL_BLEND);
            glStencilMask(1 << depth);
            glStencilFunc(GL_NEVER, 1 << depth, 0xFF);
            break;

        case RENDER_MASK_CLEAR:
            glDisable(GL_BLEND);
            glStencilMask(1 << depth);
            glStencilFunc(GL_NEVER, 0, 0xFF);
            break;
    }
}

void gl_renderer_draw(Renderer *r) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->count * sizeof( Triangle ), r->triangles);
    glDrawArrays(GL_TRIANGLES, 0, r->count * 3);
    r->count = 0;
}

