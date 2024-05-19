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
#include "graphics_internal.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

IGraphics *graphics_new_graphics_hub(int num_pages) {
    IGraphics *hub = NEW_STRUCT(IGraphics);
    hub->len_pages = num_pages;
    hub->num_pages = 0;

    hub->pages = NEW_ARRAY(hub->len_pages, IPage *);
    for (int i = 0; i < hub->len_pages; i++) {
        hub->pages[i] = NULL;
    }

    for (int i = 0; i < CHROMA_LAYERS; i++) {
        hub->time[i] = 0;
        hub->current_page[i] = 0;
    }

    return hub;
}

void graphics_hub_free_page(IGraphics *hub, int page_num) {
    if (page_num < 0 || page_num >= hub->len_pages) {
        log_file(LogWarn, "Graphics", "Page num %d out of range", page_num);
        return;
    }

    graphics_free_page(hub->pages[page_num]);
    hub->pages[page_num] = NULL;
    hub->num_pages--;
}

void graphics_free_graphics_hub(IGraphics *hub) {
    for (int i = 0; i < hub->num_pages; i++) {
        graphics_free_page(hub->pages[i]);
    }

    free(hub->pages);
    free(hub);
}

IPage *graphics_hub_add_page(IGraphics *hub, int num_geo, int num_keyframe, int id) {
    if (id < 0 || id >= hub->len_pages) {
        log_file(LogMessage, "Graphics", "Resizing graphics hub");

        int new_length = MAX(2 * hub->len_pages, id + 1);
        IPage **new_hub = NEW_ARRAY(new_length, IPage *);
        for (int i = 0; i < new_length; i++) {
            new_hub[i] = NULL;
        }

        for (int i = 0; i < hub->len_pages; i++) {
            new_hub[i] = hub->pages[i];
        }

        free(hub->pages);
        hub->pages = new_hub;
        hub->len_pages = new_length;
    }

    IPage *page = graphics_new_page(num_geo, num_keyframe);
    page->temp_id = id;
    hub->pages[id] = page;
    hub->num_pages++;

    return page;
}

IPage *graphics_hub_get_page(IGraphics *hub, int page_num) {
    if (!WITHIN(page_num, 0, hub->len_pages - 1)) {
        log_file(LogWarn, "Graphics", "Page num %d out of range", page_num);
        return NULL;
    }

    return hub->pages[page_num];
}

int graphics_hub_get_current_page_num(IGraphics *hub, int layer) {
    return hub->current_page[layer];
}

void graphics_hub_set_current_page_num(IGraphics *hub, int page_num, int layer) {
    hub->current_page[layer] = page_num;
}

void graphics_hub_set_time(IGraphics *hub, float time, int layer) {
    hub->time[layer] = time;
}

float graphics_hub_get_time(IGraphics *hub, int layer) {
    return hub->time[layer];
}
