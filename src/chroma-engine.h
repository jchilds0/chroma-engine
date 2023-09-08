/*
 * chroma-engine.h 
 */

#ifndef CHROMA_CHROMA_ENGINE
#define CHROMA_CHROMA_ENGINE

#include <raylib.h>

#define FRAMERATE     30

#define TIMEOUT       -1
#define CLOSE_SOCKET  -2

int start_tcp_server(char *, int);
int listen_for_client(int);
int recieve_message(int, char *);

typedef struct {
    char        type[30];
    int         pos_x;
    int         pos_y;
    int         width;
    int         height;
} render_object;

#endif // !CHROMA_CHROMA_ENGINE
