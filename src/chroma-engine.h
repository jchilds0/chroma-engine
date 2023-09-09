/*
 * chroma-engine.h 
 */

#ifndef CHROMA_CHROMA_ENGINE
#define CHROMA_CHROMA_ENGINE

#include <raylib.h>

#define CHROMA_FRAMERATE              30

#define CHROMA_TIMEOUT                -1
#define CHROMA_CLOSE_SOCKET           -2

#define CHROMA_TEXT                   2
#define CHROMA_PIXELS                 3
#define CHROMA_END_CONNECTION         4

int start_tcp_server(char *, int);
int listen_for_client(int);
int recieve_message(int, char *);

typedef struct {
    int         pos_x;
    int         pos_y;
    Color       color;
} render_object;

#endif // !CHROMA_CHROMA_ENGINE
