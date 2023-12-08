/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#include <GL/glew.h>

#define MAX_BUF_SIZE                  512

typedef enum {
    BLANK,
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
    int pos_x;
    int pos_y;
    GLfloat color[4];
    char buf[MAX_BUF_SIZE];
} Chroma_Text;

typedef struct {
    Chroma_Rectangle *rect;
    Chroma_Text text[2];
    int num_rect;
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
