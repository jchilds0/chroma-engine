/*
 * chroma-engine.h 
 */

#ifndef CHROMA_CHROMA_ENGINE
#define CHROMA_CHROMA_ENGINE

#include <raylib.h>
#include <stdlib.h>

#define NEW_STRUCT(struct_type)       (struct_type *) malloc((size_t) sizeof( struct_type ))
#define NEW_ARRAY(n, struct_type)     (struct_type *) malloc((size_t) (n) * sizeof( struct_type ))
#define WITHIN(a, x, y)               ((x <= a) && (a <= y))

#define CHROMA_FRAMERATE              30

#define CHROMA_TIMEOUT                -1
#define CHROMA_CLOSE_SOCKET           -2
#define CHROMA_MESSAGE                -3

#define MAX_BUF_SIZE                  512

#define END_OF_CONN                   1
#define END_OF_MESSAGE                2
#define ANIMATE_ON                    3
#define CONTINUE                      4
#define ANIMATE_OFF                   5

int start_tcp_server(char *, int);
int listen_for_client(int);
int recieve_message(int, char *);

typedef struct {
    int pos_x;
    int pos_y;
    int width;
    int height;
    Color color;
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

/* page.c */
Page *init_page(int);
void free_page(Page *);
void set_rect(Page *, int, int, int, int, int, Color);
void set_page_attr(Page *, char *, char *);
void set_page_attr_int(Page *, char *, int);
void animate_on_page(Graphics *, int);
void continue_page(Graphics *, int);
void animate_off_page(Graphics *, int);

/* graphic.c */
Graphics *init_hub(int);
void free_hub(Graphics *);
int add_graphic(Graphics *, Page *);

#endif // !CHROMA_CHROMA_ENGINE
