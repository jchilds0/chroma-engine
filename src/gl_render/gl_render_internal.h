/*
 *
 */

#ifndef GL_RENDER_INTERNAL
#define GL_RENDER_INTERNAL

#include <GL/glew.h>
#include <GL/gl.h>
#include "geometry.h"

#define SHADER_PATH       "/home/josh/Documents/projects/chroma-engine/src/gl_render/"

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

/* gl_text.c */
void gl_text_init_buffers(void);
void gl_text_init_shaders(void);
void gl_text_cache_characters(void);
void gl_draw_text(IGeometry *text);

#endif // !GL_RENDER_INTERNAL
