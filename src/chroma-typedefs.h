/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#include <GL/glew.h>

#define MAX_BUF_SIZE      512

typedef enum {
    BLANK = 0,
    END_OF_CONN,
    END_OF_MESSAGE,
    ANIMATE_ON,
    CONTINUE,
    ANIMATE_OFF,
} Action;

/* log.c */
#define LOG_PARSER        1

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
typedef enum {
    RECT,
    CIRCLE,
    ANNULUS,
    TEXT,
} ChromaGraphics;

typedef struct {
    int     pos_x;
    int     pos_y;
    int     width;
    int     height;
    GLfloat color[4];
} ChromaRectangle;

typedef struct {
    int     center_x;
    int     center_y;
    int     radius;
    GLfloat color[4];
} ChromaCircle;

typedef struct {
    int     center_x;
    int     center_y;
    int     inner_radius;
    int     outer_radius;
    GLfloat color[4];
} ChromaAnnulus;

typedef struct {
    int       pos_x;
    int       pos_y;
    char      buf[MAX_BUF_SIZE];
    char      do_transform[MAX_BUF_SIZE];
    GLfloat   transform[16];
    GLfloat   color[4];
} ChromaText;

typedef struct {
    int               num_rect;
    ChromaRectangle   *rect;

    int               num_text;
    ChromaText        *text;

    int               num_circle;
    ChromaCircle      *circle;

    int               num_annulus;
    ChromaAnnulus     *annulus;

    float             mask_time;
    float             clock_time;
    ChromaRectangle   mask;
    void              (*page_animate)(int);
    void              (*page_continue)(int);
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
