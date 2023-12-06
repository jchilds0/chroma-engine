/*
 * Function declarations for Chroma Engine
 */

#ifndef CHROMA_PROTOTYPES
#define CHROMA_PROTOTYPES

#include "chroma-typedefs.h"
#include <gtk-3.0/gtk/gtk.h>

void free_engine(void);

/* log.c */
void start_log(EngineType);
void log_to_file(LogType, char *, ...);

/* engine.c */
void engine_window(void);

/* tcp_server.c */
int start_tcp_server(char *, int);
int listen_for_client(int);
int recieve_message(int, char *);

/* page.c */
Page *init_page(int);
void free_page(Page *);
void set_rect(Page *, int, int, int, int, int);
void set_page_attr(Page *, char *, char *);
void set_page_attr_int(Page *, char *, int);
void animate_on_page(Graphics *, int);
void continue_page(Graphics *, int);
void animate_off_page(Graphics *, int);

/* graphic.c */
Graphics *init_hub(int);
void free_hub(Graphics *);
int add_graphic(Graphics *, Page *);

/* preview.c */
void preview_window(int);

/* renderer.c */
void renderer(cairo_t *);

#endif // !CHROMA_PROTOTYPES
