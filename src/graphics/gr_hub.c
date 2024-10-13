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

#include "chroma-engine.h"
#include "graphics.h"
#include "graphics_internal.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

void graphics_new_graphics_hub(IGraphics *hub, int num_pages) {
    hub->capacity = DA_INIT_CAPACITY;
    hub->count = 0;
    hub->items = NEW_ARRAY(hub->capacity, IPage);

    for (int i = 0; i < MAX_ASSETS; i++) {
        hub->img[i].data = NULL;
    }

    ARENA_INIT(&hub->arena, GIGABYTES((uint64_t) 4));
}

void graphics_hub_new_page(IGraphics *hub, int num_geo, int max_keyframe, int temp_id) {
    int i;

    if ((i = graphics_hub_get_page(hub, temp_id)) >= 0) {
        log_file(LogMessage, "Graphics", "Replacing page %d", temp_id);
        graphics_free_page(&hub->items[i]);
        IPage *page = &hub->items[i];

        graphics_init_page(page, num_geo, max_keyframe);
    } else {
        IPage temp_page;
        ARENA_INIT(&temp_page.arena, MAX_PAGE_SIZE);
        temp_page.temp_id = temp_id;
        graphics_init_page(&temp_page, num_geo, max_keyframe);

        DA_APPEND(hub, temp_page);
    } 
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

