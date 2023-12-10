/*
 * Header for page submodule
 */

#include "chroma-engine.h"

/* page.c */
Page *init_page(int, int);
void free_page(Page *);
void page_set_color(GLfloat *, GLuint, GLuint, GLuint, GLuint);

/* rect.c */
void page_rect_set_attr(Chroma_Rectangle *rect, char *attr, char *value);

/* text */
void page_text_set_attr(Chroma_Text *text, char *attr, char *value);
