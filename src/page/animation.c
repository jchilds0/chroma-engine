/*
 *
 */

#include "chroma-typedefs.h"
#include "geometry.h"
#include "page.h"
#include <GL/gl.h>

int animate_left_to_right(uint page_num, float time) {
    Page *page = engine.hub->pages[page_num];
    IGeometry *mask = page->geometry[page->mask_index];
    IGeometry *bg = page->geometry[page->bg_index];

    int bg_pos_x  = geometry_get_int_attr(bg, "pos_x");
    int bg_pos_y  = geometry_get_int_attr(bg, "pos_y");
    int bg_width  = geometry_get_int_attr(bg, "width");
    int bg_height = geometry_get_int_attr(bg, "height");

    geometry_set_int_attr(mask, "pos_x", bg_pos_x + time * bg_width);
    geometry_set_int_attr(mask, "pos_y", bg_pos_y);
    geometry_set_int_attr(mask, "width", (1.1 - time) * bg_width);
    geometry_set_int_attr(mask, "height", bg_height);
    
    return 1;
}

int animate_clock_tick(uint page_num, float time) {
    return 1;
}

int animate_none(uint page_num, float time) {
    return 1;
}
