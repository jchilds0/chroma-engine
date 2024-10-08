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
#include <sys/mman.h>

#include "log.h"

#define NEW_STRUCT(struct_type)       (struct_type *) malloc((size_t) sizeof( struct_type ))
#define NEW_ARRAY(n, struct_type)     (struct_type *) malloc((size_t) (n) * sizeof( struct_type ))

#define KILOBYTES(value)              1024 * (value)
#define MEGABYTES(value)              1024 * KILOBYTES(value)
#define GIGABYTES(value)              1024 * MEGABYTES(value)

// Arena Macros
typedef struct {
    size_t            size;
    size_t            allocd;
    void              *memory;
} Arena;

#define ARENA_ALLOC(arena, struct_type) ({                                                 \
        size_t struct_size = sizeof( struct_type );                                        \
        if ((arena)->allocd + struct_size >= (arena)->size) {                              \
            log_file(LogError, "System", "Arena out of memory " __FILE__ ":%d", __LINE__); \
        }                                                                                  \
        struct_type *ptr = (arena)->memory + (arena)->allocd;                              \
        (arena)->allocd += struct_size;                                                    \
        ptr;                                                                               \
    })

#define ARENA_ARRAY(arena, count, struct_type) ({                                          \
        size_t struct_size = (size_t) count * sizeof( struct_type );                       \
        if ((arena)->allocd + struct_size >= (arena)->size) {                              \
            log_file(LogError, "System", "Arena out of memory " __FILE__ ":%d", __LINE__); \
        }                                                                                  \
        struct_type *ptr = (arena)->memory + (arena)->allocd;                              \
        (arena)->allocd += struct_size;                                                    \
        ptr;                                                                               \
    })

// Dynamic Array Macros
#define DA_INIT_CAPACITY                 8192
#define DA_REALLOC(oldptr, old_sz, new_sz) realloc(oldptr, new_sz)
#define DA_APPEND(da, item)                                                                              \
    do {                                                                                                 \
        if ((da)->count >= (da)->capacity) {                                                             \
            size_t new_capacity = (da)->capacity * 2;                                                    \
            if (new_capacity == 0) {                                                                     \
                new_capacity = DA_INIT_CAPACITY;                                                         \
            }                                                                                            \
                                                                                                         \
            size_t item_size = sizeof( (da)->items[0] );                                                 \
                                                                                                         \
            (da)->items = DA_REALLOC((da)->items, (da)->capacity * item_size, new_capacity * item_size); \
            (da)->capacity = new_capacity;                                                               \
        }                                                                                                \
                                                                                                         \
        (da)->items[(da)->count++] = (item);                                                             \
    } while (0)


// Linked List Macros
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

#define INSTALL_DIR                   "/home/josh/programming/chroma-engine/"

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
