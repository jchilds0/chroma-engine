/*
 * gr_hub_archive.c
 *
 * Temporary c file to initialise a template
 * graphics hub. Exists in place of a file 
 * format and parser for storing the graphics
 * hub.
 *
 */

#include "graphics_internal.h"
#include "geometry.h"

void graphics_hub_load_example(IGraphics *hub) {
    IPage *page;
    IGeometry *geo, *bg;

    /* red box */
    page = graphics_hub_add_page(hub, 5, 0);

    // bg
    bg = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(bg, "color", "255 0 0 255");

    // text
    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 1;
    page->mask_index = 4;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* orange box */
    page = graphics_hub_add_page(hub);

    bg = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(bg, "color", "255 165 0 255");

    // text
    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 1;
    page->mask_index = 4;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* blue box */
    page = graphics_hub_add_page(hub);

    // bg
    bg = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(bg, "color", "2 132 130 255");
    geometry_set_attr(bg, "rounding", "20");

    // circle
    geo = graphics_page_add_geometry(page, bg, "circle");
    geometry_set_attr(geo, "color", "255 255 255 255");

    // text
    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 1;
    page->mask_index = 5;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* clock box */
    page = graphics_hub_add_page(hub);

    IGeometry *div;

    // bg
    bg = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(bg, "color", "5 68 94 255");
    geometry_set_attr(bg, "rounding", "10");

    // circle
    geo = graphics_page_add_geometry(page, bg, "circle");
    geometry_set_attr(geo, "color", "255 255 255 255");

    // left split 
    div = graphics_page_add_geometry(page, bg, "rect");
    geometry_set_attr(div, "color", "255 255 255 255");

    // Team 1
    geo = graphics_page_add_geometry(page, div, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // Score 1
    geo = graphics_page_add_geometry(page, div, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mid split
    div = graphics_page_add_geometry(page, div, "rect");
    geometry_set_attr(div, "color", "255 255 255 255");

    // Team 2
    geo = graphics_page_add_geometry(page, div, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // Score 2
    geo = graphics_page_add_geometry(page, div, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // right split 
    div = graphics_page_add_geometry(page, div, "rect");
    geometry_set_attr(div, "color", "255 255 255 255");

    // clock 
    geo = graphics_page_add_geometry(page, div, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // clock text
    geo = graphics_page_add_geometry(page, div, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 1;
    page->mask_index = 12;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_continue = graphics_animate_clock_tick;
    page->page_animate_off = graphics_animate_left_to_right;

    /* white circle */
    page = graphics_hub_add_page(hub);

    // circle
    geo = graphics_page_add_geometry(page, NULL, "circle");
    geometry_set_attr(geo, "color", "255 255 255 255");

    /* graph */
    page = graphics_hub_add_page(hub);

    // bg 
    bg = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(bg, "color", "2 132 130 255");
    geometry_set_attr(bg, "rounding", "20");

    // graph
    geo = graphics_page_add_geometry(page, bg, "graph");
    geometry_set_attr(geo, "graph_type", "step");
    geometry_set_attr(geo, "color", "255 255 255 255");

    // text 
    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask 
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 1;
    page->mask_index = 4;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* ticker */
    page = graphics_hub_add_page(hub);

    // bg 
    bg = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(bg, "color", "2 132 130 255");

    // extra box 
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "5 68 94 255");
    geometry_set_attr(geo, "rounding", "50");

    // text 
    geo = graphics_page_add_geometry(page, bg, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask 
    geo = graphics_page_add_geometry(page, NULL, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 1;
    page->mask_index = 4;
    page->page_animate_on = graphics_animate_up;
    page->page_animate_off = graphics_animate_up;
}
