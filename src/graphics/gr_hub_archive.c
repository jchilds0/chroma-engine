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
    IGeometry *geo;

    /* red box */
    page = graphics_hub_add_page(hub);

    // bg
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "255 0 0 255");

    // text
    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 3;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* orange box */
    page = graphics_hub_add_page(hub);

    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "255 165 0 255");

    // text
    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 3;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* blue box */
    page = graphics_hub_add_page(hub);

    // bg
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "2 132 130 255");

    // circle
    geo = graphics_page_add_geometry(page, "circle");
    geometry_set_attr(geo, "color", "255 255 255 255");

    // text
    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 4;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* clock box */
    page = graphics_hub_add_page(hub);

    // bg
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 150 255");

    // text
    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 2;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_continue = graphics_animate_clock_tick;
    page->page_animate_off = graphics_animate_left_to_right;

    /* white circle */
    page = graphics_hub_add_page(hub);

    // circle
    geo = graphics_page_add_geometry(page, "circle");
    geometry_set_attr(geo, "color", "255 255 255 255");

    /* graph */
    page = graphics_hub_add_page(hub);

    // bg 
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "2 132 130 255");

    // graph
    geo = graphics_page_add_geometry(page, "graph");
    geometry_set_attr(geo, "graph_type", "step");
    geometry_set_attr(geo, "color", "255 255 255 255");

    // text 
    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask 
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 3;
    page->page_animate_on = graphics_animate_left_to_right;
    page->page_animate_off = graphics_animate_left_to_right;

    /* ticker */
    page = graphics_hub_add_page(hub);

    // bg 
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "2 132 130 255");

    // text 
    geo = graphics_page_add_geometry(page, "text");
    geometry_set_attr(geo, "color", "255 255 255 255");
    geometry_set_attr(geo, "scale", "1.0");

    // mask 
    geo = graphics_page_add_geometry(page, "rect");
    geometry_set_attr(geo, "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 2;
    page->page_animate_on = graphics_animate_up;
    page->page_animate_off = graphics_animate_up;
}
