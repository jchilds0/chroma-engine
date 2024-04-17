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

IPage *graphics_new_page(int num_geo, int num_keyframe) {
    IPage *page = NEW_STRUCT(IPage);
    page->len_geometry = num_geo + 1;
    page->num_geometry = 0;
    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);

    for (int i = 0; i < page->len_geometry; i++) {
        page->geometry[i] = NULL;
    }

    page->len_keyframe = num_keyframe;
    page->num_keyframe = 0;
    page->keyframe = NEW_ARRAY(page->len_keyframe, Keyframe);
    page->k_value = NULL;
    page->attr_keyframe = NEW_ARRAY(page->len_geometry * NUM_ATTR, unsigned char);

    memset(page->attr_keyframe, 0, page->len_geometry * NUM_ATTR);

    // root 
    IGeometry *geo = graphics_page_add_geometry(page, 0, "rect");
    geometry_set_int_attr(geo, "pos_x", 0);
    geometry_set_int_attr(geo, "pos_y", 0);

    return page;
}

void graphics_page_clean_page(IPage *page) {
    if (page == NULL) {
        log_file(LogError, "Graphics", "Attempted to clean null page");
    }
    for (int i = 0; i < page->num_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue;
        }

        geometry_clean_geo(page->geometry[i]);
    }
}

IGeometry *graphics_page_add_geometry(IPage *page, int id, char *type) {
    if (id < 0 || id >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Can't add geometry to page, id %d out of range", id);
        return NULL;
    }

    IGeometry *geo = geometry_create_geometry(type);
    page->geometry[id] = geo;
    page->num_geometry++;
    return geo;
}

void graphics_page_set_bg_index(IPage *page, int index) {
    if (index < 0 || index >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Background index %d may be out of range", index);
    } 

    page->bg_index = index;
}

void graphics_page_set_mask_index(IPage *page, int index) {
    if (index < 0 || index >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Mask index %d may be out of range", index);
    } 

    page->mask_index = index;
}

IGeometry *graphics_page_get_geometry(IPage *page, int geo_num) {
    return page->geometry[geo_num];
}

void graphics_free_page(IPage *page) {
    if (page == NULL) {
        return;
    }

    for (int i = 0; i < page->num_geometry; i++) {
        geometry_free_geometry(page->geometry[i]);
    }

    free(page->geometry);
    free(page);
}

int graphics_page_num_geometry(IPage *page) {
    return page->num_geometry;
}

static void graphics_page_update_child_geometry(IPage *page, unsigned int node) {
    IGeometry *parent, *child;
    int child_num, parent_num;

    child = page->geometry[node];
    parent_num = geometry_get_int_attr(child, "parent");
    parent = page->geometry[parent_num];

    int parent_x = geometry_get_int_attr(parent, "pos_x");
    int parent_y = geometry_get_int_attr(parent, "pos_y");
    int rel_x = geometry_get_int_attr(child, "rel_x");
    int rel_y = geometry_get_int_attr(child, "rel_y");

    geometry_set_int_attr(child, "pos_x", parent_x + rel_x);
    geometry_set_int_attr(child, "pos_y", parent_y + rel_y);

    for (int i = 1; i < page->num_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue ;
        }

        child_num = geometry_get_int_attr(page->geometry[i], "parent");
        if (child_num != node) {
            // geo is not a child of the current child
            continue;
        }

        graphics_page_update_child_geometry(page, i);
    }
}

void graphics_page_update_geometry(IPage *page) {
    int parent_num;

    for (int i = 1; i < page->num_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue;
        }

        parent_num = geometry_get_int_attr(page->geometry[i], "parent");

        if (parent_num == 0) {
            graphics_page_update_child_geometry(page, i);
        }
    }
}

void graphics_page_interpolate_geometry(IPage *page, int index, int width) {
    if (page->max_keyframe == 0) {
        log_file(LogMessage, "Graphics", "No keyframes, skipping interpolation");
        return;
    }

    IGeometry *geo;
    int next_value, k_index;
    int frame_width = width / page->max_keyframe;
    int frame_start = index / frame_width;
    int frame_index = index % frame_width;

    log_file(LogMessage, "Graphics", "Interpolating page %d at keyframe %d and index %d", page->temp_id, frame_start, frame_index);

    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        for (int attr = 0; attr < NUM_ATTR; attr++) {
            if (!page->attr_keyframe[geo_id * NUM_ATTR + attr]) {
                continue;
            }

            k_index = geo_id * (page->max_keyframe * NUM_ATTR) 
                + attr * page->max_keyframe + frame_start;

            if (frame_start == page->max_keyframe) {
                continue;
            }

            geo = page->geometry[geo_id];
            next_value = graphics_keyframe_interpolate_int(
                page->k_value[k_index],
                page->k_value[k_index + 1], 
                frame_index,
                frame_width
            );
            geometry_set_int_attr(geo, ATTR[attr], next_value);
            log_file(LogMessage, "Graphics", "Set geo %d attr %s to %d", geo_id, ATTR[attr], next_value);
        }
    }
}
