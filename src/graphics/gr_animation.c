/*
 * gr_animation.c
 *
 * Animates a page by updating the geometries of the 
 * page according to the time attribute, which is 
 * stored in the graphics hub struct and passed to 
 * the animation when called.
 *
 */

#include "graphics_internal.h"
#include "geometry.h"

int graphics_animate_left_to_right(IPage *page, float time) {
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

int graphics_animate_right_to_left(IPage *page, float time) {
    return 1;
}

int graphics_animate_up(IPage *page, float time) {
    IGeometry *mask = page->geometry[page->mask_index];
    IGeometry *bg = page->geometry[page->bg_index];

    int bg_pos_x  = geometry_get_int_attr(bg, "pos_x");
    int bg_pos_y  = geometry_get_int_attr(bg, "pos_y");
    int bg_width  = geometry_get_int_attr(bg, "width");
    int bg_height = geometry_get_int_attr(bg, "height");

    geometry_set_int_attr(mask, "pos_x", bg_pos_x);
    geometry_set_int_attr(mask, "pos_y", bg_pos_y + time * bg_height);
    geometry_set_int_attr(mask, "width", bg_width);
    geometry_set_int_attr(mask, "height", (1.1 - time) * bg_height);
    
    return 1;
}

int graphics_animate_none(IPage *page, float time) {
    return 1;
}
