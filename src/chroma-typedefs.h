/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#define MAX_BUF_SIZE      512

#include "page.h"

typedef unsigned int      uint;

typedef enum {
    BLANK = 0,
    END_OF_CONN,
    END_OF_MESSAGE,
    ANIMATE_ON,
    CONTINUE,
    ANIMATE_OFF,
} Action;

/* graphics structs */
typedef struct {
    int num_pages;
    int size_of_pages;
    int current_page;
    float time;
    Page **pages;
} Graphics;

typedef struct {
    int         socket;
    int         port;
    Graphics    *hub;
} Engine;

extern Engine engine;
extern int action;
extern int page_num;

#endif // !CHROMA_TYPEDEFS
