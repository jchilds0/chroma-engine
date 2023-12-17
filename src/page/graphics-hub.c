/*
 *
 */

#include "chroma-engine.h"
#include "chroma-prototypes.h"
#include "chroma-typedefs.h"
#include "page.h"

int add_graphic(Graphics *, Page *);

void page_make_hub(void) {
    // red box
    Page *page = init_page(10);

    // bg
    page->geometry[0] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[0], "color", "255 0 0 255");

    // text
    page->geometry[1] = geometry_create_geometry("text");
    page->geometry[2] = geometry_create_geometry("text");
    geometry_set_attr(page->geometry[1], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[1], "scale", "1.0");
    geometry_set_attr(page->geometry[2], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[2], "scale", "1.0");

    // mask
    page->geometry[3] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[3], "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 3;
    page->num_geometry = 4;
    page->page_animate_on = animate_left_to_right;
    page->page_animate_off = animate_left_to_right;
    add_graphic(engine.hub, page); 

    // orange box
    page = init_page(10);

    // bg 
    page->geometry[0] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[0], "color", "255 165 0 255");

    // text
    page->geometry[1] = geometry_create_geometry("text");
    page->geometry[2] = geometry_create_geometry("text");
    geometry_set_attr(page->geometry[1], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[1], "scale", "1.0");
    geometry_set_attr(page->geometry[2], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[2], "scale", "1.0");

    // mask
    page->geometry[3] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[3], "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 3;
    page->num_geometry = 4;
    page->page_animate_on = animate_left_to_right;
    page->page_animate_off = animate_left_to_right;
    add_graphic(engine.hub, page); 

    // blue box
    page = init_page(10);

    // bg
    page->geometry[0] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[0], "color", "0 0 255 255");

    // circle
    page->geometry[1] = geometry_create_geometry("circle");
    geometry_set_attr(page->geometry[1], "color", "255 255 255 255");

    // text
    page->geometry[2] = geometry_create_geometry("text");
    page->geometry[3] = geometry_create_geometry("text");
    geometry_set_attr(page->geometry[2], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[2], "scale", "1.0");
    geometry_set_attr(page->geometry[3], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[3], "scale", "1.0");

    // mask
    page->geometry[4] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[4], "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 4;
    page->num_geometry = 5;
    page->page_animate_on = animate_left_to_right;
    page->page_animate_off = animate_left_to_right;
    add_graphic(engine.hub, page); 

    // clock box
    page = init_page(10);

    // bg
    page->geometry[0] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[0], "color", "0 0 150 255");

    // text
    page->geometry[1] = geometry_create_geometry("text");
    geometry_set_attr(page->geometry[1], "color", "255 255 255 255");
    geometry_set_attr(page->geometry[1], "scale", "1.0");

    // mask
    page->geometry[2] = geometry_create_geometry("rect");
    geometry_set_attr(page->geometry[2], "color", "0 0 0 255");

    page->bg_index = 0;
    page->mask_index = 2;
    page->num_geometry = 3;
    page->page_animate_on = animate_left_to_right;
    page->page_continue = animate_clock_tick;
    page->page_animate_off = animate_left_to_right;
    add_graphic(engine.hub, page); 

    // white circle
    page = init_page(10);

    // circle
    page->geometry[0] = geometry_create_geometry("circle");
    geometry_set_attr(page->geometry[0], "color", "255 255 255 255");

    page->num_geometry = 1;
    page->page_animate_on = animate_none;
    page->page_continue = animate_none;
    page->page_animate_off = animate_none;
    add_graphic(engine.hub, page); 
}

Graphics *init_hub(int num_pages) {
    Graphics *hub = NEW_STRUCT(Graphics);
    hub->pages = NEW_ARRAY(num_pages, Page *);
    hub->size_of_pages = num_pages;
    hub->num_pages = 1;
    
    hub->pages[0] = init_page(0);
    hub->current_page = 0;

    return hub;
}

void free_hub(Graphics *hub) {
    for (int i = 1; i < hub->num_pages; i++) {
        free_page(hub->pages[i]);
    }

    free(hub);
}

int add_graphic(Graphics *hub, Page *page) {
    if (hub->num_pages == hub->size_of_pages) {
        return 0;
    }

    hub->pages[hub->num_pages] = page;
    hub->num_pages++;
    return 1;
}

