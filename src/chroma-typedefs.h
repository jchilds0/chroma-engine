/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#include "chroma-macros.h"
#include "glib.h"
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
    int     layer;
    Action  action;
} PageStatus;

/* graphics structs */
typedef struct {
    int client_sock;
    char buf[PARSE_BUF_SIZE];
    int buf_ptr;
} Client;

typedef struct {
    GMutex           lock;
    int              server_port;
    unsigned char    render_perf;
    int              hub_socket;
    char             hub_addr[MAX_BUF_SIZE];
    IGraphics        hub;
} Engine;

extern Engine engine;

#endif // !CHROMA_TYPEDEFS
