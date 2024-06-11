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
#include <stdio.h>
#include <string.h>

IPage *graphics_new_page(int num_geo, int num_keyframe) {
    IPage *page = NEW_STRUCT(IPage);
    page->len_geometry = num_geo + 1;
    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);

    for (int i = 0; i < page->len_geometry; i++) {
        page->geometry[i] = NULL;
    }

    page->len_keyframe = num_keyframe;
    page->num_keyframe = 0;
    page->keyframe = NEW_ARRAY(page->len_keyframe, Keyframe);
    page->k_value = NULL;
    page->attr_keyframe = NEW_ARRAY(page->len_geometry * GEO_NUM, unsigned char);

    memset(page->attr_keyframe, 0, page->len_geometry * GEO_NUM);

    // root 
    IGeometry *geo = graphics_page_add_geometry(page, 0, RECT);
    geometry_set_int_attr(geo, GEO_POS_X, 0);
    geometry_set_int_attr(geo, GEO_POS_Y, 0);
    geometry_set_int_attr(geo, GEO_WIDTH, 1920);
    geometry_set_int_attr(geo, GEO_HEIGHT, 1080);

    geometry_set_int_attr(geo, GEO_X_LOWER, 0);
    geometry_set_int_attr(geo, GEO_X_UPPER, 1920);
    geometry_set_int_attr(geo, GEO_Y_LOWER, 0);
    geometry_set_int_attr(geo, GEO_Y_UPPER, 1080);

    return page;
}

IGeometry *graphics_page_add_geometry(IPage *page, int id, int type) {
    if (id < 0 || id >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Can't add geometry to page, id %d out of range", id);
        return NULL;
    }

    IGeometry *geo = geometry_create_geometry(type);
    page->geometry[id] = geo;
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

    for (int i = 0; i < page->len_geometry; i++) {
        geometry_free_geometry(page->geometry[i]);
    }

    free(page->geometry);
    free(page);
}

int graphics_page_num_geometry(IPage *page) {
    return page->len_geometry;
}

int graphics_page_num_frames(IPage *page) {
    return page->max_keyframe;
}

static void graphics_geometry_update_absolute_position(IGeometry *parent, IGeometry *child) {
    int parent_x = geometry_get_int_attr(parent, GEO_POS_X);
    int parent_y = geometry_get_int_attr(parent, GEO_POS_Y);
    int rel_x = geometry_get_int_attr(child, GEO_REL_X);
    int rel_y = geometry_get_int_attr(child, GEO_REL_Y);

    geometry_set_int_attr(child, GEO_POS_X, parent_x + rel_x);
    geometry_set_int_attr(child, GEO_POS_Y, parent_y + rel_y);
}

static void graphics_geometry_update_mask(IGeometry *parent, IGeometry *child) {
    int x_lower = geometry_get_int_attr(parent, GEO_X_LOWER);
    int x_upper = geometry_get_int_attr(parent, GEO_X_UPPER);
    int y_lower = geometry_get_int_attr(parent, GEO_Y_LOWER);
    int y_upper = geometry_get_int_attr(parent, GEO_Y_UPPER);

    int pos_x = geometry_get_int_attr(parent, GEO_POS_X);
    int pos_y = geometry_get_int_attr(parent, GEO_POS_Y);
    int width = geometry_get_int_attr(parent, GEO_WIDTH);
    int height = geometry_get_int_attr(parent, GEO_HEIGHT);

    geometry_set_int_attr(child, GEO_X_LOWER, MAX(x_lower, pos_x));
    geometry_set_int_attr(child, GEO_X_UPPER, MIN(x_upper, pos_x + width));
    geometry_set_int_attr(child, GEO_Y_LOWER, MAX(y_lower, pos_y));
    geometry_set_int_attr(child, GEO_Y_UPPER, MIN(y_upper, pos_y + height));
}

static void graphics_page_update_child_geometry(IPage *page, unsigned int node) {
    IGeometry *parent, *child;
    int child_num, parent_num;

    child = page->geometry[node];
    parent_num = geometry_get_int_attr(child, GEO_PARENT);
    parent = page->geometry[parent_num];

    graphics_geometry_update_absolute_position(parent, child);
    graphics_geometry_update_mask(parent, child);

    for (int i = 1; i < page->len_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue ;
        }

        child_num = geometry_get_int_attr(page->geometry[i], GEO_PARENT);
        if (child_num != node) {
            // geo is not a child of the current child
            continue;
        }

        graphics_page_update_child_geometry(page, i);
    }
}

void graphics_page_update_geometry(IPage *page) {
    int parent_num;

    for (int i = 1; i < page->len_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue;
        }

        parent_num = geometry_get_int_attr(page->geometry[i], GEO_PARENT);

        if (parent_num == 0) {
            graphics_page_update_child_geometry(page, i);
        }
    }
}

void graphics_page_interpolate_geometry(IPage *page, int index, int width) {
    if (page->max_keyframe == 1) {
        //log_file(LogMessage, "Graphics", "No keyframes, skipping interpolation");
        return;
    }

    IGeometry *geo;
    int next_value, k_index;
    int frame_start = index / width;
    int frame_index = index % width;

    //log_file(LogMessage, "Graphics", "Interpolating page %d at keyframe %d and index %d", page->temp_id, frame_start, frame_index);

    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        geo = page->geometry[geo_id];
        if (geo == NULL) {
            continue;
        }

        for (int attr = 0; attr < GEO_NUM; attr++) {
            if (!page->attr_keyframe[geo_id * GEO_NUM + attr]) {
                continue;
            }

            k_index = geo_id * (page->max_keyframe * GEO_NUM) 
                + attr * page->max_keyframe + frame_start;

            if (frame_start == page->max_keyframe - 1) {
                next_value = page->k_value[k_index];
            } else {
                next_value = graphics_keyframe_interpolate_int(
                    page->k_value[k_index], page->k_value[k_index + 1], 
                    frame_index, width
                );
            }

            geometry_set_int_attr(geo, attr, next_value);
            //log_file(LogMessage, "Graphics", "Set geo %d attr %d to %d", geo_id, attr, next_value);
        }
    }
}
