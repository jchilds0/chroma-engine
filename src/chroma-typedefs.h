/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#include <GL/glew.h>

#define MAX_BUF_SIZE                  512

typedef enum {
    BLANK = 0,
    END_OF_CONN,
    END_OF_MESSAGE,
    ANIMATE_ON,
    CONTINUE,
    ANIMATE_OFF,
} Action;

/* log.c */
typedef enum {
    LogMessage,
    LogWarn,
    LogError,
} LogType;

typedef enum {
    LogEngine,
    LogPreview
} EngineType;

/* graphics structs */
typedef struct {
    int     pos_x;
    int     pos_y;
    int     width;
    int     height;
    GLfloat color[4];
} Chroma_Rectangle;

typedef struct {
    int       pos_x;
    int       pos_y;
    char      buf[MAX_BUF_SIZE];
    char      do_transform[MAX_BUF_SIZE];
    GLfloat   transform[16];
    GLfloat   color[4];
} Chroma_Text;

typedef struct {
    int num_rect;
    int num_text;
    Chroma_Rectangle *rect;
    Chroma_Text *text;
    float mask_time;
    float clock_time;
    Chroma_Rectangle mask;
    void (*page_animate)(int);
    void (*page_continue)(int);
} Page;

typedef struct {
    Page **pages;
    int num_pages;
    int size_of_pages;
    int current_page;
} Graphics;

typedef struct {
    int         socket;
    int         port;
    Graphics    *hub;
} Engine;

extern Engine engine;
extern Action action;
extern int page_num;

#endif // !CHROMA_TYPEDEFS
