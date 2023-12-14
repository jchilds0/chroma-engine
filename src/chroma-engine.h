/*
 * Main header for Chroma Engine 
 *
 * All .c files should include this header.
 */

#ifndef CHROMA_ENGINE
#define CHROMA_ENGINE

#include <gdk/gdk.h>
#include <gtk/gtkx.h>
#include <gtk/gtk.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "gl_math.h"

#define NEW_STRUCT(struct_type)       (struct_type *) malloc((size_t) sizeof( struct_type ))
#define NEW_ARRAY(n, struct_type)     (struct_type *) malloc((size_t) (n) * sizeof( struct_type ))
#define WITHIN(a, x, y)               ((x <= a) && (a <= y))

#define CHROMA_FRAMERATE              30

#endif // !CHROMA_ENGINE
