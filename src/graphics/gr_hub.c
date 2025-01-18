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

#include "arena.h"
#include "glib.h"
#include "graphics.h"
#include "graphics_internal.h"

void graphics_new_graphics_hub(IGraphics *hub, int num_pages) {
    g_mutex_init(&hub->lock);

    hub->capacity = DA_INIT_CAPACITY;
    hub->count = 0;
    hub->items = NEW_ARRAY(hub->capacity, IPage *);

    for (int i = 0; i < MAX_ASSETS; i++) {
        hub->img[i].data = NULL;
    }

    ARENA_INIT(&hub->arena, GIGABYTES((uint64_t) 4));
}

IPage *graphics_hub_new_page(IGraphics *hub, int num_geo, int max_keyframe, int temp_id) {
    IPage *page;

    page = graphics_hub_get_page(hub, temp_id);

    if (page != NULL) {
        log_file(LogMessage, "Graphics", "Replacing page %d", temp_id);
        graphics_page_clear(page);
    } else {

        g_mutex_lock(&hub->lock);

        page = ARENA_ALLOC(&hub->arena, IPage);
        g_mutex_init(&page->lock);

        DA_APPEND(hub, page);
        g_mutex_unlock(&hub->lock);

        graphics_page_init_arena(page);
    } 

    graphics_init_page(page, temp_id, num_geo, max_keyframe);
    return page;
}

IPage *graphics_hub_get_page(IGraphics *hub, int temp_id) {
    IPage *page = NULL;
    g_mutex_lock(&hub->lock);

    for (size_t i = 0; i < hub->count; i++) {
        if (hub->items[i]->temp_id != temp_id) {
            continue;
        }

        page = hub->items[i];
    }

    g_mutex_unlock(&hub->lock);
    return page;
}

void graphics_free_graphics_hub(IGraphics *hub) {
    if (hub == NULL) {
        return;
    }

    g_mutex_lock(&hub->lock);
    for (size_t i = 0; i < hub->count; i++) {
        IPage *page = hub->items[i];
        graphics_page_free_page(page);
    }

    free(hub->items);
    g_mutex_unlock(&hub->lock);
}
