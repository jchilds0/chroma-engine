/*
 *
 */

#include "chroma-engine.h"
#include "page.h"

Page *init_page(uint num_geo) {
    Page *page = NEW_STRUCT(Page);
    page->len_geometry = num_geo;
    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);

    page->page_animate_on = animate_none;
    page->page_continue = animate_none;
    page->page_animate_off = animate_none;
    return page;
}

void free_page(Page *page) {
    for (int i = 0; i < page->num_geometry; i++) {
        geometry_free_geometry(page->geometry[i]);
    }

    free(page->geometry);
    free(page);
}

