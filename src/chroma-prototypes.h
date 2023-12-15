/*
 * Function declarations for Chroma Engine
 */

#ifndef CHROMA_PROTOTYPES
#define CHROMA_PROTOTYPES

#include "chroma-typedefs.h"
#include <GL/glew.h>
#include <gtk-3.0/gtk/gtk.h>

void free_engine(void);

/* log.c */
void log_start(EngineType);
void log_file(LogType, char *, ...);

/* engine.c */
void engine_window(void);

/* tcp_server.c */
int parser_tcp_start_server(char *, int);

/* page.c */
void page_set_page_attrs(Page *page, char *attr, char *value);
void page_animate_on(int page_num);
void page_continue(int page_num);
void page_animate_off(int page_num);

/* graphic.c */
void page_make_hub(void);
Graphics *init_hub(int);
void free_hub(Graphics *);

/* preview.c */
void preview_window(int);

/* parser.c */
void parser_read_socket(int *, Action *);

/* gl_renderer.c */ 
void gl_realize(GtkWidget *);
gboolean gl_render(GtkGLArea *, GdkGLContext *);

void gl_text_render(ChromaText *text, float scale);
void gl_rect_render(ChromaRectangle *rect);
void gl_circle_render(ChromaCircle *circle);
void gl_annulus_render(ChromaAnnulus *annulus);

#endif // !CHROMA_PROTOTYPES
