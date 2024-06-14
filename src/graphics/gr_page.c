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

IPage *graphics_new_page(int num_geo, int max_keyframe) {
    IPage *page = NEW_STRUCT(IPage);
    page->len_geometry = num_geo + 1;
    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);

    for (int i = 0; i < page->len_geometry; i++) {
        page->geometry[i] = NULL;
    }

    page->max_keyframe = MAX(max_keyframe, 1) + 1;
    int n = page->max_keyframe * page->len_geometry * GEO_NUM;

    page->keyframe_graph = graphics_new_graph(n);

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

void graphics_page_interpolate_geometry(IPage *page, int index, int width) {
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
            k_index = INDEX(geo_id, attr, frame_start, GEO_NUM, page->max_keyframe);
            if (!page->keyframe_graph->exists[k_index]) {
                continue;
            }

            if (!geometry_is_int_attr(attr)) {
                continue;
            }

            if (frame_start == page->max_keyframe - 1) {
                next_value = page->keyframe_graph->value[k_index];
            } else {
                next_value = graphics_keyframe_interpolate_int(
                    page->keyframe_graph->value[k_index], 
                    page->keyframe_graph->value[k_index + 1], 
                    frame_index, width
                );
            }

            geometry_set_int_attr(geo, attr, next_value);
            //log_file(LogMessage, "Graphics", "Set geo %d attr %d to %d", geo_id, attr, next_value);
        }
    }
}
