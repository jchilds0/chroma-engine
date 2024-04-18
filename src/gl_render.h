/*
 * Header for the Open GL renderer module.
 *
 * Exposes the functions
 *
 *    void gl_realize(GtkWidget *);
 *    gboolean gl_render(GtkGLArea *, GdkGLContext *);
 *
 * which should be connected to the 'realize' and 
 * 'render' signals of a GtkGLArea object 
 * respectively. gl_realize() initializes the various
 * buffers and shaders for gl rendering and gl_render()
 * waits for an action and page number from the parser.
 * 
 * Once it has an action and page number, it updates the
 * animation frame of the page, and then calls the 
 * relevant gl render functions for each IGeometry in 
 * the page.
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

extern unsigned int gl_text_text_width(char *text, float scale);
extern unsigned int gl_text_text_height(char *text, float scale);

extern int action[];
extern int page_num[];

#endif // !CHROMA_GL_RENDERER
