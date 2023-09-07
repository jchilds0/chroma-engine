/*
 * chroma-engine.h 
 */

#ifndef CHROMA_CHROMA_ENGINE
#define CHROMA_CHROMA_ENGINE

#include <raylib.h>

void open_tcp_server(void);

typedef struct {
    char        type[30];
    int         pos_x;
    int         pos_y;
    int         width;
    int         height;
} render_object;

#endif // !CHROMA_CHROMA_ENGINE
