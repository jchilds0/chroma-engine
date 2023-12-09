/*
 *
 */

#include "chroma-engine.h"

void make_pages(void) {
    // red box
    Page *page = init_page(1, 2);
    set_color(&page->rect[0].color[0], 255, 0, 0, 255);
    add_graphic(engine.hub, page);

    // orange box
    page = init_page(1, 2);
    set_color(&page->rect[0].color[0], 255, 165, 0, 255);
    add_graphic(engine.hub, page);

    // blue box
    page = init_page(1, 2);
    set_color(&page->rect[0].color[0], 0, 0, 255, 255);
    add_graphic(engine.hub, page);

    // clock box
    page = init_page(1, 1);
    set_color(&page->rect[0].color[0], 0, 0, 150, 255);
    add_graphic(engine.hub, page);
}

Graphics *init_hub(int num_pages) {
    Graphics *hub = NEW_STRUCT(Graphics);
    hub->pages = NEW_ARRAY(num_pages, Page *);
    hub->size_of_pages = num_pages;
    hub->num_pages = 1;

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

