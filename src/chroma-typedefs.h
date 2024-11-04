/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#include "chroma-engine.h"
#include "graphics.h"

typedef enum {
    BLANK = 0,
    END_OF_CONN,
    END_OF_MESSAGE,
    ANIMATE_ON,
    CONTINUE,
    ANIMATE_OFF,
    UPDATE,
} Action;

typedef struct {
    int     temp_id;
    int     frame_num;
    int     layer;
    Action  action;
} PageStatus;

/* graphics structs */
typedef struct {
    unsigned char active;
    int           server_socket;
    int           server_port;

    int           hub_socket;
    char          hub_addr[MAX_BUF_SIZE];

    IGraphics     hub;
} Engine;

extern Engine engine;

#endif // !CHROMA_TYPEDEFS
