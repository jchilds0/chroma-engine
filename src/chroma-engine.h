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

#define NEW_STRUCT(struct_type)       (struct_type *) malloc((size_t) sizeof( struct_type ))
#define NEW_ARRAY(n, struct_type)     (struct_type *) malloc((size_t) (n) * sizeof( struct_type ))

#define INSERT_BEFORE(new, old)     {                               \
                                        (new)->next = (old);        \
                                        (new)->prev = (old)->prev;  \
                                        (new)->prev->next = (new);  \
                                        (new)->next->prev = (new);  \
                                    }

#define INSERT_AFTER(new, old)      {                               \
                                        (new)->prev = (old);        \
                                        (new)->next = (old)->next;  \
                                        (new)->prev->next = (new);  \
                                        (new)->next->prev = (new);  \
                                    }

#define REMOVE_NODE(node)           {                                       \
                                        (node)->next->prev = (node)->prev;  \
                                        (node)->prev->next = (node)->next;  \
                                    }

#define WITHIN(a, x, y)               ((x <= a) && (a <= y))
#define MIN(a, b)                     (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                     (((a) > (b)) ? (a) : (b))
#define INDEX(x, y, z, y_len, z_len)  x * (y_len * z_len) + y * z_len + z

#define INSTALL_DIR                   "/home/josh/Documents/projects/chroma-engine/"

#define CHROMA_FRAMERATE              30

/*
 * Layer categories
 *  0: (Reserved) preview renders all graphics to this layer,
 *  1: Top left graphics,
 *  2: Lower frame graphics,
 *  3: Ticker,
 *
 */

#define CHROMA_LAYERS                 5 
#define MAX_BUF_SIZE                  512

#endif // !CHROMA_ENGINE
