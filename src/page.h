/*
 * Header for page submodule
 */

#ifndef CHROMA_PAGE
#define CHROMA_PAGE

#include "geometry.h"

typedef struct {
    unsigned int      mask_index;
    unsigned int      bg_index;
    unsigned int      num_geometry;
    unsigned int      len_geometry;
    IGeometry         **geometry;
    int               (*page_animate_on)(unsigned int, float);
    int               (*page_continue)(unsigned int, float);
    int               (*page_animate_off)(unsigned int, float);
} Page;

/* page.c */
Page *init_page(unsigned int num_geo);
void free_page(Page *);

/* animation.c */
int animate_left_to_right(unsigned int page_num, float time);
int animate_right_to_left(unsigned int page_num, float time);
int animate_clock_tick(unsigned int page_num, float time);
int animate_none(unsigned int page_num, float time);

#endif // !CHROMA_PAGE
