/*
 *
 */

#ifndef CHROMA_GL_RENDERER
#define CHROMA_GL_RENDERER

#include "chroma-engine.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <math.h>

#define SHADER_PATH       "/home/josh/Documents/projects/chroma-engine/src/gl_render/"

#define DEG_TO_RAD(theta)     (theta * M_PI / 180)

#define GL_MATH_ROTATE_X(theta) {\
    1, 0,           0,            0, \
    0, cosf(theta), -sinf(theta), 0, \
    0, sinf(theta), cosf(theta),  0, \
    0, 0,           0,            1}

#define GL_MATH_ROTATE_Z(theta)         {\
    cosf(theta),-sinf(theta), 0, 0, \
    sinf(theta), cosf(theta), 0, 0, \
    0          , 0          , 1, 0, \
    0          , 0          , 0, 1}

#define GL_MATH_ORTHO(left, right, bottom, top, zNear, zFar)    {\
    2 / (right - left), 0,                  0,                  (right + left) / (left - right), \
    0,                  2 / (top - bottom), 0,                  (top + bottom) / (bottom - top), \
    0,                  0,                  2 / (zNear - zFar), (zFar + zNear) / (zNear - zFar), \
    0,                  0,                  0,                  1}

#define FOV_TO_F(fov)      (1.0 / tanf(fov / 2.0))

#define GL_MATH_PERSPECTIVE(fov, aspect, zNear, zFar)    {\
    FOV_TO_F(fov) / (aspect), 0.0,             0.0,                             0.0, \
    0.0,                      FOV_TO_F(fov),   0.0,                             0.0, \
    0.0,                      0.0,             (zFar + zNear) / (zNear - zFar), (2 * zFar * zNear) / (zNear - zFar), \
    0.0,                      0.0,             -1.0,                            0.0}

#define GL_MATH_TRANSLATE(x, y, z) {\
    1.0, 0.0, 0.0, x, \
    0.0, 1.0, 0.0, y, \
    0.0, 0.0, 1.0, z, \
    0.0, 0.0, 0.0, 1.0}

#define GL_MATH_ID        { 1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1 }

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


