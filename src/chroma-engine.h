/*
 * chroma-engine.h 
 */

#ifndef CHROMA_CHROMA_ENGINE
#define CHROMA_CHROMA_ENGINE

#include <raylib.h>

#define CHROMA_FRAMERATE              30

#define CHROMA_TIMEOUT                -1
#define CHROMA_CLOSE_SOCKET           -2

#define MAX_BUF_SIZE                  100

#define RENDER_PIXELS                 2
#define RENDER_TEXT                   3
#define END_OF_PIXEL                  4
#define END_OF_FRAME                  5
#define END_OF_CON                    6

int start_tcp_server(char *, int);
int listen_for_client(int);
int recieve_message(int, char *);

typedef struct {
    int         pos_x;
    int         pos_y;
    Color       color;
} RenderObject;

#endif // !CHROMA_CHROMA_ENGINE
