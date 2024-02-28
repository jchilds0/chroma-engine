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
#include "geometry.h"

#define SHADER_PATH       "src/gl_render/"

/* gl_renderer.c */
char  *gl_renderer_get_shader_file(char *filename);
GLuint gl_renderer_create_shader(int type, const char *src);
GLuint gl_renderer_create_program(GLuint vertex, GLuint fragment);
void gl_renderer_set_scale(GLuint program);

/* gl_rect.c */
void gl_rectangle_init_buffers(void);
void gl_rectangle_init_shaders(void);
void gl_draw_rectangle(IGeometry *rect);

/* gl_circle.c */
void gl_circle_init_buffers(void);
void gl_circle_init_shaders(void);
void gl_draw_circle(IGeometry *circle);

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

/* gl_video.c */
void gl_video_init_buffers(void);
void gl_video_init_shaders(void);

#endif // !GL_RENDER_INTERNAL
