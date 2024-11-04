/*
 * gl_shape.c
 *
 * Render simple shapes to a renderer.
 *
 */

#include "gl_render_internal.h"

/*
 * Rectangle vertex layout (corners 
 * filled in by rounding)
 *
 *  12  13 --- 14 15
 *   8 - 9     10-11  
 *   |             |
 *   4 - 5     6 - 7 
 *   0   1 --- 2   3
 */

void gl_draw_rectangle(Renderer *r, GeometryRect *rect) {
    float pos_x = rect->geo.pos.x;
    float pos_y = rect->geo.pos.y;

    int width = rect->width;
    int height = rect->height;
    int round = MIN(rect->rounding, MIN(width / 2, height / 2));

    vec2 points[16] = {
        // row 0
        {pos_x,                 pos_y},
        {pos_x + round,         pos_y},
        {pos_x + width - round, pos_y},
        {pos_x + width,         pos_y},

        // row 1
        {pos_x,                 pos_y + round},
        {pos_x + round,         pos_y + round},
        {pos_x + width - round, pos_y + round},
        {pos_x + width,         pos_y + round},

        // row 2
        {pos_x,                 pos_y + height - round},
        {pos_x + round,         pos_y + height - round},
        {pos_x + width - round, pos_y + height - round},
        {pos_x + width,         pos_y + height - round},

        // row 3
        {pos_x,                 pos_y + height},
        {pos_x + round,         pos_y + height},
        {pos_x + width - round, pos_y + height},
        {pos_x + width,         pos_y + height},
    };

    vec2 u0 = {0, 0};
    vec4 color = rect->color;

    gl_renderer_triangle(r, points[1], points[2], points[5], color, color, color, u0, u0, u0);
    gl_renderer_triangle(r, points[2], points[5], points[6], color, color, color, u0, u0, u0);

    gl_renderer_triangle(r, points[4], points[7], points[8], color, color, color, u0, u0, u0);
    gl_renderer_triangle(r, points[8], points[7], points[11], color, color, color, u0, u0, u0);

    gl_renderer_triangle(r, points[9], points[10], points[13], color, color, color, u0, u0, u0);
    gl_renderer_triangle(r, points[10], points[13], points[14], color, color, color, u0, u0, u0);

    GeometryCircle circle = {{CIRCLE, 0, 0}, 0, round, 0, 2 * M_PI, color};

    circle.geo.pos = points[5];
    circle.start_angle = M_PI;
    circle.end_angle = 3 * M_PI / 2;
    gl_draw_circle(r, &circle);

    circle.geo.pos = points[6];
    circle.start_angle = 3 * M_PI / 2;
    circle.end_angle = 2 * M_PI;
    gl_draw_circle(r, &circle);

    circle.geo.pos = points[9];
    circle.start_angle = M_PI / 2;
    circle.end_angle = M_PI;
    gl_draw_circle(r, &circle);

    circle.geo.pos = points[10];
    circle.start_angle = 0;
    circle.end_angle = M_PI / 2;
    gl_draw_circle(r, &circle);
}
void gl_draw_polygon(Renderer *r, GeometryPolygon *p) {
    vec2 center = {0, 0};

    if (p->num_vertices == 0) {
        return;
    }

    // compute polygon center
    for (int i = 0; i < p->num_vertices; i++) {
        center = ADD2(center, p->vertex[i]);
    }

    center = (vec2){center.x / p->num_vertices, center.y / p->num_vertices};
    center = ADD2(p->geo.pos, center);

    vec4 color = p->color;
    vec2 p0, p1, zero = {0, 0};
    for (int i = 1; i < p->num_vertices; i++) {
        p0 = p->vertex[i - 1];
        p1 = p->vertex[i];
        gl_renderer_triangle(r, center, p0, p1, color, color, color, zero, zero, zero);
    }
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

void gl_draw_circle(Renderer *r, GeometryCircle *c) {
    vec2 center = c->geo.pos;
    vec4 color = c->color;
    vec2 u0 = {0, 0};

    int n = gl_circle_tri_num(c->outer_radius, c->start_angle, c->end_angle);
    double theta = (double)(c->end_angle - c->start_angle) / n;

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

    vec2 p0, p1, p2, p3;
    vec2 inner, outer;
    double cos_theta, sin_theta;

    for (int i = 0; i < n; i++) {
        cos_theta = cosf(theta * i + c->start_angle);
        sin_theta = sinf(theta * i + c->start_angle);

        inner = (vec2){c->inner_radius * cos_theta, c->inner_radius * sin_theta};
        p0 = ADD2(center, inner);

        outer = (vec2){c->outer_radius * cos_theta, c->outer_radius * sin_theta};
        p1 = ADD2(center, outer);

        cos_theta = cosf(theta * (i + 1) + c->start_angle);
        sin_theta = sinf(theta * (i + 1) + c->start_angle);

        inner = (vec2){c->inner_radius * cos_theta, c->inner_radius * sin_theta};
        p2 = ADD2(center, inner);

        outer = (vec2){c->outer_radius * cos_theta, c->outer_radius * sin_theta};
        p3 = ADD2(center, outer);

        gl_renderer_triangle(r, p0, p1, p2, color, color, color, u0, u0, u0);
        gl_renderer_triangle(r, p1, p2, p3, color, color, color, u0, u0, u0);
    }
}

