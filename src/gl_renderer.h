/*
 *
 */

#ifndef CHROMA_GL_RENDERER
#define CHROMA_GL_RENDERER

#include <GL/glew.h>
#include <GL/gl.h>
#include <gtk/gtk.h>

#include <ft2build.h>
#include FT_FREETYPE_H

extern void gl_realize(GtkWidget *);
extern gboolean gl_render(GtkGLArea *, GdkGLContext *);

#endif // !CHROMA_GL_RENDERER
