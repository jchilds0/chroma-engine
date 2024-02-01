/*
 * gr_page.c 
 *
 * Functions for the IPage struct, which contains
 * the geometry of the page, how to update the 
 * geometry when it is animated on, continued or
 * animated off.
 *
 */

#include "graphics_internal.h"
#include "chroma-engine.h"
#include "geometry.h"
#include "log.h"
#include <string.h>

IPage *graphics_new_page(int num_geo) {
    IPage *page = NEW_STRUCT(IPage);
    page->len_geometry = num_geo;
    page->num_geometry = 0;
    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);
    page->parent_geo = NEW_ARRAY(page->len_geometry, unsigned int);

    page->page_animate_on = graphics_animate_none;
    page->page_continue = graphics_animate_none;
    page->page_animate_off = graphics_animate_none;
    return page;
}

IGeometry *graphics_page_add_geometry(IPage *page, int id, int parent, char *type) {
    if (id < 0 || id >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Can't add geometry to page, id %d out of range", id);
        return NULL;
    }

    IGeometry *geo = geometry_create_geometry(type);
    page->geometry[id] = geo;
    page->parent_geo[id] = parent;
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

void graphics_page_update_animation(IPage *page, char *anim, float time) {
    if (strncmp(anim, "animate_on", 10) == 0) {
        page->page_animate_on(page, time);
    } else if (strncmp(anim, "continue", 8) == 0) {
        page->page_continue(page, time);
    } else if (strncmp(anim, "animate_off", 11) == 0) {
        page->page_animate_off(page, time);
    } else {
        log_file(LogWarn, "Graphics", "Unknown animation type (%s)", anim);
    }
}

static void graphics_page_update_child_geometry(IPage *page, unsigned int node) {
    IGeometry *parent = page->geometry[page->parent_geo[node]];
    IGeometry *child = page->geometry[node];

    int parent_x = geometry_get_int_attr(parent, "pos_x");
    int parent_y = geometry_get_int_attr(parent, "pos_y");
    int rel_x = geometry_get_int_attr(child, "rel_x");
    int rel_y = geometry_get_int_attr(child, "rel_y");

    geometry_set_int_attr(child, "pos_x", parent_x + rel_x);
    geometry_set_int_attr(child, "pos_y", parent_y + rel_y);

    for (int i = 0; i < page->num_geometry; i++) {
        if (page->parent_geo[i] == node) {
            graphics_page_update_child_geometry(page, i);
        }
    }
}

void graphics_page_update_geometry(IPage *page) {
    for (int i = 1; i < page->num_geometry; i++) {
        if (page->parent_geo[i] == 0) {
            graphics_page_update_child_geometry(page, i);
        }
    }
}

