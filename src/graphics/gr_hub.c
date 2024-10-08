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

IGraphics *graphics_new_graphics_hub(int num_pages) {
    IGraphics *hub = NEW_STRUCT(IGraphics);
    hub->capacity = DA_INIT_CAPACITY;
    hub->count = 0;
    hub->items = NEW_ARRAY(hub->capacity, IPage *);

    for (int i = 0; i < hub->capacity; i++) {
        hub->items[i] = NULL;
    }

    return hub;
}

void graphics_free_graphics_hub(IGraphics *hub) {
    for (int i = 0; i < hub->count; i++) {
        graphics_free_page(hub->items[i]);
    }

    free(hub->items);
    free(hub);
}

void graphics_hub_add_page(IGraphics *hub, IPage *page) {
    int i;
    if ((i = graphics_hub_get_page(hub, page->temp_id)) >= 0) {
        log_file(LogMessage, "Graphics", "Replacing page");
        graphics_free_page(hub->items[i]);
        hub->items[i] = page;
        return;
    }

    DA_APPEND(hub, page);
}

int graphics_hub_get_page(IGraphics *hub, int temp_id) {
    int index = -1;
    for (size_t i = 0; i < hub->count; i++) {
        if (hub->items[i] == NULL) {
            continue;
        }

        if (hub->items[i]->temp_id != temp_id) {
            continue;
        }

        index = i;
    }

    return index;
}

