/*
 * gr_hub.c
 *
 * Functions for the IGraphics struct which 
 * contains the graphics hub of the engine. 
 * The hub contains a number of pages, and 
 * each page can be rendered to distinct 
 * layers, so the graphics hub contains the 
 * current page and time of each layer.
 *
 */

#include "graphics_internal.h"

void graphics_new_graphics_hub(IGraphics *hub, int num_pages) {
    hub->capacity = DA_INIT_CAPACITY;
    hub->count = 0;
    hub->items = NEW_ARRAY(hub->capacity, IPage);

    for (int i = 0; i < MAX_ASSETS; i++) {
        hub->img[i].data = NULL;
    }

    ARENA_INIT(&hub->arena, GIGABYTES((uint64_t) 4));
}

IPage *graphics_hub_new_page(IGraphics *hub, int num_geo, int max_keyframe, int temp_id) {
    IPage *page;
    int i;

    if ((i = graphics_hub_get_page(hub, temp_id)) >= 0) {
        log_file(LogMessage, "Graphics", "Replacing page %d", temp_id);
        graphics_page_clear(&hub->items[i]);
        page = &hub->items[i];

        graphics_init_page(page, num_geo, max_keyframe);
    } else {
        IPage temp_page;
        DA_APPEND(hub, temp_page);
        page = &hub->items[hub->count - 1];

        ARENA_INIT(&page->arena, MAX_PAGE_SIZE);
        page->temp_id = temp_id;

        graphics_init_page(page, num_geo, max_keyframe);
    } 

    return page;
}

int graphics_hub_get_page(IGraphics *hub, int temp_id) {
    int index = -1;
    for (size_t i = 0; i < hub->count; i++) {
        if (hub->items[i].temp_id != temp_id) {
            continue;
        }

        index = i;
    }

    return index;
}

void graphics_free_graphics_hub(IGraphics *hub) {
    if (hub == NULL) {
        return;
    }

    for (size_t i = 0; i < hub->count; i++) {
        IPage *page = &hub->items[i];
        graphics_page_free_page(page);
    }

    free(hub->items);
}
