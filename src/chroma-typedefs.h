/*
 * Type definitions for Chroma Engine
 */

#ifndef CHROMA_TYPEDEFS
#define CHROMA_TYPEDEFS

#define MAX_BUF_SIZE                  512

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
    int pos_x;
    int pos_y;
    int width;
    int height;
    //Color color;
} Chroma_Rectangle;

typedef struct {
    int pos_x;
    int pos_y;
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

#endif // !CHROMA_TYPEDEFS
