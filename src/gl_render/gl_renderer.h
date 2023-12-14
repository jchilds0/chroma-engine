/*
 *
 */

#ifndef CHROMA_GL_RENDERER
#define CHROMA_GL_RENDERER

#include "chroma-engine.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#define SHADER_PATH       "/home/josh/Documents/projects/chroma-engine/src/gl_render/"

/* gl_renderer.c */
char  *gl_renderer_get_shader_file(char *filename);
GLuint gl_renderer_create_shader(int type, const char *src);
GLuint gl_renderer_create_program(GLuint vertex, GLuint fragment);
void gl_renderer_set_scale(GLuint program);

/* gl_text.c */ 
void gl_text_init_buffers(void);
void gl_text_init_shaders(void);
void gl_text_cache_characters(void);

/* gl_rectange.c */ 
void gl_rect_init_buffers(void);
void gl_rect_init_shaders(void);

#endif // !CHROMA_GL_RENDERER


