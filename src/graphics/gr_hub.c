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
    hub->items = NEW_ARRAY(hub->capacity, IPage *);

    for (int i = 0; i < hub->capacity; i++) {
        hub->items[i] = NULL;
    }

    for (int i = 0; i < MAX_ASSETS; i++) {
        hub->img[i].data = NULL;
    }

    hub->arena.allocd = 0;
    hub->arena.size = GIGABYTES((uint64_t)4);
    hub->arena.memory = mmap(NULL, hub->arena.size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);
    if (hub->arena.memory == MAP_FAILED) {
        log_file(LogError, "Graphics", "Unable to allocate image: %s", strerror(errno));
    }

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

