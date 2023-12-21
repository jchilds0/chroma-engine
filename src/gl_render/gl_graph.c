/*
 * gl_graph.c
 *
 * Setup and render a graph described by a 
 * GeometryGraph in a GL context.
 *
 */

#include "chroma-engine.h"
#include "gl_math.h"
#include "gl_render_internal.h"
#include "geometry.h"
#include "log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <X11/Xutil.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint program;

static GLfloat *vertices     = NULL;
static unsigned int *indices = NULL;

static GLfloat axis_v[]      = { 
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 0.0f,
};
static unsigned int axis_i[] = {
    0, 1,
    0, 2,
};
static int num_nodes = -1;

void gl_graph_init_buffers(void) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // bind the vertex array object
    glBindVertexArray(vao);
    glBindVertexArray(0);
}

void gl_graph_init_shaders(void) {
    char *vertexSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.vs.glsl");
    char *fragmentSource = gl_renderer_get_shader_file(SHADER_PATH "glshape-gl.fs.glsl");

    GLuint vertex = gl_renderer_create_shader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = gl_renderer_create_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program = gl_renderer_create_program(vertex, fragment);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);
    glLineWidth(3.0);

    free(vertexSource);
    free(fragmentSource);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

static void gl_draw_axis(vec2 pos, vec2 offset) {
    // find max for axis calc
    int x_min = INT_MAX, x_max = INT_MIN;
    int y_min = INT_MAX, y_max = INT_MIN;
    for (int i = 0; i < num_nodes; i++) {
        x_min = MIN(x_min, (int)vertices[3 * i]);
        x_max = MAX(x_max, (int)vertices[3 * i]);
        y_min = MIN(y_min, (int)vertices[3 * i + 1]);
        y_max = MAX(y_max, (int)vertices[3 * i + 1]);
    }

    // (0, 0) 
    axis_v[0] = pos.x + offset.x;
    axis_v[1] = pos.y + offset.y;

    // x axis end
    axis_v[3] = x_max + offset.x;
    axis_v[4] = pos.y + offset.y;

    // y axis end
    axis_v[6] = pos.x + offset.x;
    axis_v[7] = y_max + offset.y;

    // draw axis
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( axis_v ), axis_v, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( axis_i ), axis_i, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, 0);
}

static void gl_graph_gen_line(GeometryGraph *g, vec2 pos, vec2 offset) {
    // number of nodes has changed, realloc arrays
    if (num_nodes != g->num_nodes) {
        free(vertices);
        free(indices);

        num_nodes = g->num_nodes;
        vertices = NEW_ARRAY(3 * num_nodes, GLfloat);
        indices = NEW_ARRAY(2 * num_nodes, unsigned int);
    }

    for (int i = 0; i < num_nodes; i++) {
        vertices[3 * i]     = pos.x + offset.x + g->nodes[i].x;
        vertices[3 * i + 1] = pos.y + offset.y + g->nodes[i].y;
        vertices[3 * i + 2] = 0.0f;
        
        if (i < num_nodes - 1) {
            indices[2 * i]     = i;
            indices[2 * i + 1] = i + 1;
        } else {
            indices[2 * i]     = 0;
            indices[2 * i + 1] = 0;
        }
    }
}

static void gl_graph_gen_bezier(GeometryGraph *g, vec2 pos, vec2 offset) {
    // number of nodes has changed, realloc arrays
    if (num_nodes != g->num_nodes) {
        free(vertices);
        free(indices);

        num_nodes = g->num_nodes;
        vertices = NEW_ARRAY(3 * num_nodes, GLfloat);
        indices = NEW_ARRAY(2 * num_nodes, unsigned int);
    }
}

static void gl_graph_gen_point(GeometryGraph *g, vec2 pos, vec2 offset) {
    int node_x, node_y;

    // number of nodes has changed, realloc arrays
    if (num_nodes != g->num_nodes) {
        free(vertices);
        free(indices);

        num_nodes = g->num_nodes;
        vertices = NEW_ARRAY(3 * num_nodes, GLfloat);
        indices = NEW_ARRAY(2 * num_nodes, unsigned int);
    }

    for (int i = 0; i < num_nodes; i++) {
        node_x = g->nodes[i].x;
        node_y = g->nodes[i].y;

        vertices[3 * i]     = pos.x + offset.x + node_x;
        vertices[3 * i + 1] = pos.y + offset.y + node_y;
        vertices[3 * i + 2] = 0.0f;
        
        if (i < num_nodes - 1) {
            indices[2 * i]     = i;
            indices[2 * i + 1] = i + 1;
        } else {
            indices[2 * i]     = 0;
            indices[2 * i + 1] = 0;
        }
    }
}

static void gl_graph_gen_step(GeometryGraph *g, vec2 pos, vec2 offset) {
    // number of nodes has changed, realloc arrays
    if (num_nodes != 2 * g->num_nodes) {
        free(vertices);
        free(indices);

        num_nodes = 2 * g->num_nodes;
        vertices = NEW_ARRAY(3 * num_nodes, GLfloat);
        indices = NEW_ARRAY(2 * num_nodes, unsigned int);
    }

    for (int i = 0; i < g->num_nodes; i++) {
        // x = current point, y = last point
        vertices[6 * i]     = pos.x + offset.x + g->nodes[i].x;
        vertices[6 * i + 1] = pos.y + offset.y + g->nodes[MAX(i - 1, 0)].y;
        vertices[6 * i + 2] = 0.0f;

        // current point
        vertices[6 * i + 3] = pos.x + offset.x + g->nodes[i].x;
        vertices[6 * i + 4] = pos.y + offset.y + g->nodes[i].y;
        vertices[6 * i + 5] = 0.0f;
        
        indices[4 * i]     = MAX(2 * i - 2, 0);
        indices[4 * i + 1] = MAX(2 * i - 1, 0);
        indices[4 * i + 2] = MAX(2 * i - 1, 0);
        indices[4 * i + 3] = 2 * i;
    }
}

void gl_draw_graph(IGeometry *graph) {
    GeometryGraph *geo_graph = (GeometryGraph *)graph;
    char value[GEO_BUF_SIZE];
    GLfloat r, g, b, a;
    vec2 pos = {geo_graph->pos_x, geo_graph->pos_y};
    vec2 offset = {20, 20};

    memset(value, '\0', sizeof value);
    geometry_get_attr(graph, "color", value);
    sscanf(value, "%f %f %f %f", &r, &g, &b, &a);

    // generate vertices for the graph
    switch (geo_graph->graph_type) {
        case LINE:
            gl_graph_gen_line(geo_graph, pos, offset);
            break;
        case BEZIER:
            gl_graph_gen_bezier(geo_graph, pos, offset);
            break;
        case POINT:
            gl_graph_gen_point(geo_graph, pos, offset);
            break;
        case STEP:
            gl_graph_gen_step(geo_graph, pos, offset);
            break;
        default:
            log_file(LogWarn, "GL Renderer", "Unknown graph type %d", geo_graph->graph_type);
    }

    gl_renderer_set_scale(program);

    glUseProgram(program);
    glBindVertexArray(vao);

    uint color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, r, g, b, a);

    gl_draw_axis(pos, offset);

    // draw graph
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof( GLfloat ) * 3 * num_nodes, vertices, GL_STATIC_DRAW);

    // bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int ) * 2 * num_nodes, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_LINES, 2 * (num_nodes + 2), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
