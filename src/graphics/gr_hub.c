/*
 *
 */

#include "chroma-engine.h"
#include "graphics_internal.h"
#include "log.h"
#include <stdio.h>

IGraphics *graphics_new_graphics_hub(void) {
    IGraphics *hub = NEW_STRUCT(IGraphics);
    hub->len_pages = 10;
    hub->num_pages = 0;

    hub->pages = NEW_ARRAY(hub->len_pages, IPage *);

    // blank page
    graphics_hub_add_page(hub);
    hub->current_page = 0;
    hub->time = 0.0f;

    return hub;
}

void graphics_free_graphics_hub(IGraphics *hub) {
    for (int i = 0; i < hub->num_pages; i++) {
        graphics_free_page(hub->pages[i]);
    }

    free(hub->pages);
    free(hub);
}

IPage *graphics_hub_add_page(IGraphics *hub) {
    if (hub->len_pages == hub->num_pages) {
        log_file(LogWarn, "Graphics", "Graphics hub out of memory");
        return 0;
    }

    IPage *page = graphics_new_page();
    hub->pages[hub->num_pages] = page;
    hub->num_pages++;

    return page;
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

void graphics_hub_set_current_page_num(IGraphics *hub, int page_num) {
    hub->current_page = page_num;
}

void graphics_hub_set_time(IGraphics *hub, float time) {
    hub->time = time;
}

float graphics_hub_get_time(IGraphics *hub) {
    return hub->time;
}
