/*
 *
 */

#include "graphics_internal.h"
#include "chroma-engine.h"
#include "geometry.h"
#include "log.h"

IPage *graphics_new_page(void) {
    IPage *page = NEW_STRUCT(IPage);
    page->len_geometry = 10;
    page->num_geometry = 0;
    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);

    page->page_animate_on = graphics_animate_none;
    page->page_continue = graphics_animate_none;
    page->page_animate_off = graphics_animate_none;
    return page;
}

IGeometry *graphics_page_add_geometry(IPage *page, char *type) {
    if (page->num_geometry == page->len_geometry) {
        log_file(LogWarn, "Graphics", "Can't add geometry to page, out of memory");
        return NULL;
    }

    IGeometry *geo = geometry_create_geometry(type);
    page->geometry[page->num_geometry] = geo;
    page->num_geometry++;

    return geo;
}

IGeometry *graphics_page_get_geometry(IPage *page, int geo_num) {
    return page->geometry[geo_num];
}

void graphics_free_page(IPage *page) {
    for (int i = 0; i < page->num_geometry; i++) {
        geometry_free_geometry(page->geometry[i]);
    }

    free(page->geometry);
    free(page);
}

int graphics_page_num_geometry(IPage *page) {
    return page->num_geometry;
}
