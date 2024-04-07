/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#include "graphics.h"

typedef enum {
    BLANK = 0,
    END_OF_CONN,
    END_OF_MESSAGE,
    ANIMATE_ON,
    CONTINUE,
    ANIMATE_OFF,
    CLEAN,
} Action;

/* graphics structs */
typedef struct {
    int         server_socket;
    int         server_port;
    int         hub_socket;
    int         hub_port;
    IGraphics   *hub;
} Engine;

extern Engine engine;

#endif // !CHROMA_TYPEDEFS
