/*
 *
 */

#ifndef PAGE_INTERNAL
#define PAGE_INTERNAL

#include "geometry.h"

typedef struct IPage {
    unsigned int      mask_index;
    unsigned int      bg_index;
    unsigned int      num_geometry;
    unsigned int      len_geometry;
    IGeometry         **geometry;
    int               (*page_animate_on)(struct IPage *page, float time);
    int               (*page_continue)(struct IPage *page, float time);
    int               (*page_animate_off)(struct IPage *page, float time);
} IPage;

typedef struct {
    unsigned int      num_pages;
    unsigned int      len_pages;
    unsigned int      current_page;
    float             time;
    IPage             **pages;
} IGraphics;

IPage *graphics_new_page(void);
void graphics_free_page(IPage *);

/* animation.c */
int graphics_animate_left_to_right(IPage *page, float time);
int graphics_animate_right_to_left(IPage *page, float time);
int graphics_animate_clock_tick(IPage *page, float time);
int graphics_animate_none(IPage *page, float time);

#endif // !PAGE_INTERNAL
