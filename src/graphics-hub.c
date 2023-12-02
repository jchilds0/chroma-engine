/*
 *
 */

#include "chroma-engine.h"
#include <raylib.h>


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

