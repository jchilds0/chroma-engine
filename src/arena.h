/*
 * arena.h
 *
 * Arena implementation
 */

#ifndef ARENA_H
#define ARENA_H

#include "log.h"
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint64_t size;
    uint64_t allocd;
    int8_t   *memory;
} Arena;

#define ARENA_INIT(arena, capacity)                                                        \
    do {                                                                                   \
        (arena)->allocd = 0;                                                               \
        (arena)->size = capacity;                                                          \
        (arena)->memory = mmap(NULL, (arena)->size, PROT_WRITE | PROT_READ,                \
                MAP_PRIVATE | MAP_ANON, 0, 0);                                             \
        log_assert((arena)->memory != MAP_FAILED,                                          \
                "System", "Unable to allocate arena: " __FILE__);                          \
    } while (0)

#define ARENA_ALLOC(arena, struct_type) ({                                                 \
        uint64_t struct_size = sizeof( struct_type );                                      \
        log_assert((arena)->allocd + struct_size < (arena)->size,                          \
                "System", "Arena out of memory " __FILE__);                                \
        struct_type *ptr = (struct_type *)((arena)->memory + (arena)->allocd);             \
        (arena)->allocd += struct_size;                                                    \
        memset(ptr, 0, struct_size);                                                       \
        ptr;                                                                               \
    })

#define ARENA_ARRAY(arena, count, struct_type) ({                                          \
        uint64_t struct_size = (uint64_t) count * sizeof( struct_type );                   \
        log_assert((arena)->allocd + struct_size < (arena)->size,                          \
                "System", "Arena out of memory " __FILE__);                                \
        struct_type *ptr = (struct_type *)((arena)->memory + (arena)->allocd);             \
        (arena)->allocd += struct_size;                                                    \
        memset(ptr, 0, struct_size);                                                       \
        ptr;                                                                               \
    })

#endif // !ARENA_H

