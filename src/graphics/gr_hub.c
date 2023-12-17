/*
 *
 */

#include "chroma-engine.h"
#include "graphics_internal.h"
#include "log.h"
#include <stdio.h>

IGraphics *graphics_new_graphics_hub(void) {
    IGraphics *hub = NEW_STRUCT(IGraphics);
    hub->size_of_pages = 10;

    hub->pages = NEW_ARRAY(hub->size_of_pages, IPage *);
    hub->num_pages = 1;
    hub->current_page = 0;

    // blank page
    hub->pages[0] = graphics_new_page();

    return hub;
}

void graphics_free_graphics_hub(IGraphics *hub) {
    for (int i = 1; i < hub->num_pages; i++) {
        graphics_free_page(hub->pages[i]);
    }

    free(hub->pages);
    free(hub);
}

int graphics_hub_add_page(IGraphics *hub) {
    if (hub->size_of_pages == hub->num_pages) {
        log_file(LogWarn, "Graphics", "Graphics hub out of memory");
        return 0;
    }

    hub->pages[hub->num_pages] = graphics_new_page();
    hub->num_pages++;
    return hub->num_pages - 1;
}

IPage *graphics_hub_get_page(IGraphics *hub, int page_num) {
    if (!WITHIN(page_num, 0, hub->num_pages - 1)) {
        log_file(LogWarn, "Graphics", "Page num %d out of range", page_num);
        return NULL;
    }

    return hub->pages[page_num];
}

int graphics_hub_get_current_page_num(IGraphics *hub) {
    return hub->current_page;
}

void graphics_hub_set_current_page(IGraphics *hub, int page_num) {
    hub->current_page = page_num;
}

void graphics_hub_set_time(IGraphics *hub, float time) {
    hub->time = time;
}

float graphics_hub_get_time(IGraphics *hub) {
    return hub->time;
}

void graphics_page_update_on(IGraphics *hub, int page_num) {

}

void graphics_page_update_cont(IGraphics *hub, int page_num) {

}

void graphics_page_update_off(IGraphics *hub, int page_num) {

}

void graphics_load_from_file(IGraphics *hub, FILE *file) {

}
