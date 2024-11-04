/*
 * gl_render_internal.h
 *
 * Header file for gl_render module source code.
 * Should not be included by files outside the 
 * /gl_render dir.
 *
 */

#ifndef GL_RENDER_INTERNAL
#define GL_RENDER_INTERNAL

#include <GL/glew.h>
#include <GL/gl.h>
#include "chroma-engine.h"
#include "geometry.h"

/* gl_renderer.c */
#define TRIANGLE_CAP      (MEGABYTES((uint64_t)128) / sizeof( Triangle ))

typedef enum {
    RENDER_DRAW_NO_MASK = 0,
    RENDER_DRAW_MASK,
    RENDER_MASK,
    RENDER_MASK_CLEAR,
} RendererOptions;

typedef enum {
    VERTEX_ATTR_POSITION = 0,
    VERTEX_ATTR_COLOR,
    VERTEX_ATTR_UV,
} VertexAttr;

typedef struct {
    vec2 pos;
    vec4 color;
    vec2 uv;
} Vertex;

typedef struct {
    Vertex v[3];
} Triangle;

typedef struct Renderer {
    GLuint vao;
    GLuint vbo;
    GLuint program;

    size_t count;
    Triangle triangles[TRIANGLE_CAP];
} Renderer;

void gl_renderer_triangle_init(Renderer *r, const char *vert_file_path, const char *frag_file_path);
void gl_renderer_use(Renderer *r);
void gl_renderer_triangle(Renderer *r, vec2 p0, vec2 p1, vec2 p2, 
                          vec4 c0, vec4 c1, vec4 c2, vec2 uv0, vec2 uv1, vec2 uv2);
void gl_renderer_mask(Renderer *r, RendererOptions opt, int depth);
void gl_renderer_draw(Renderer *r);

GLuint gl_renderer_create_shader(int type, const char *src);
GLuint gl_renderer_create_program(GLuint vertex, GLuint fragment);
void gl_renderer_set_scale(GLuint program);

extern const char *glimage_vs_glsl;
extern const char *glimage_fs_glsl;
extern const char *glrender_vs_glsl;
extern const char *glrender_fs_glsl;
extern const char *glshape_vs_glsl;
extern const char *glshape_fs_glsl;
extern const char *gltext_vs_glsl;
extern const char *gltext_fs_glsl;

/* gl_rect.c */
void gl_draw_rectangle(Renderer *r, GeometryRect *rect);

/* gl_circle.c */
void gl_draw_circle(Renderer *r, GeometryCircle *circle);

/* gl_annulus.c */
void gl_annulus_init_buffers(void);
void gl_annulus_init_shaders(void);
void gl_draw_annulus(IGeometry *annulus);

/* gl_graph.c */ 
void gl_graph_init_buffers(void);
void gl_graph_init_shaders(void);
void gl_draw_graph(IGeometry *graph);

/* gl_text.c */
void gl_text_init_buffers(void);
void gl_text_init_shaders(void);
void gl_text_cache_characters(void);
void gl_draw_text(IGeometry *text);

/* gl_image.c */
void gl_image_init_buffers(void);
void gl_image_init_shaders(void);
void gl_draw_image(IGeometry *image);

/* gl_poly.c */
void gl_polygon_init_buffers(void);
void gl_polygon_init_shaders(void);
void gl_draw_polygon(Renderer *r, GeometryPolygon *poly);

#endif // !GL_RENDER_INTERNAL
