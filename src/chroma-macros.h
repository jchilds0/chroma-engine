/*
 * chroma-macros.h
 *
 * common macros for chroma engine
 */

#ifndef CHROMA_MACROS_H
#define CHROMA_MACROS_H

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define CHROMA_FRAMERATE              120

/*
 * Layer categories
 *  0: (Reserved) preview renders all graphics to this layer,
 *  1: Top left graphics,
 *  2: Lower frame graphics,
 *  3: Ticker,
 *
 */

#define CHROMA_LAYERS                 5 

#define NEW_STRUCT(struct_type)       (struct_type *) malloc((size_t) sizeof( struct_type ))
#define NEW_ARRAY(n, struct_type)     (struct_type *) malloc((size_t) (n) * sizeof( struct_type ))

#define KILOBYTES(value)              (1024 * (value))
#define MEGABYTES(value)              (1024 * KILOBYTES(value))
#define GIGABYTES(value)              (1024 * MEGABYTES(value))

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

#define MAX_BUF_SIZE                  512

#endif // !CHROMA_MACROS_H
