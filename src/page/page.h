/*
 * Header for page submodule
 */

#include "chroma-engine.h"

/* page.c */
Page *init_page(int rect, int text, int circ, int annulus);
void free_page(Page *);
void page_set_color(GLfloat *, GLuint, GLuint, GLuint, GLuint);
int page_update_geo(char *attr, char *value);

/* rect.c */
void page_rect_set_attr(ChromaRectangle *rect, char *attr, char *value);

/* circle */
void page_circle_set_attr(ChromaCircle *circle, char *attr, char *value);

/* annulus */
void page_annulus_set_attr(ChromaAnnulus *annulus, char *attr, char *value);

/* text */
void page_text_set_attr(ChromaText *text, char *attr, char *value);

/* animation.c */
void animate_left_to_right(int page_num);
void animate_right_to_left(int page_num);
void animate_clock_tick(int page_num);
void animate_none(int page_num);
