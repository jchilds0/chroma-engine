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
#include "log.h"
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#define MAX_PAGE_SIZE     GIGABYTES((uint64_t) 1)

IPage *graphics_new_page(int num_geo, int max_keyframe) {
    IPage *page = NEW_STRUCT(IPage);
    page->arena.allocd = 0;
    page->arena.size = MAX_PAGE_SIZE;
    page->arena.memory = mmap(NULL, page->arena.size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);
    if (page->arena.memory == MAP_FAILED) {
        log_file(LogError, "Graphics", "Unable to allocate page: %s", strerror(errno));
    }

    page->len_geometry = num_geo;
    page->max_keyframe = max_keyframe;
    page->geometry = ARENA_ARRAY(&page->arena, num_geo, IGeometry *);

    for (int i = 0; i < page->len_geometry; i++) {
        page->geometry[i] = NULL;
    }

    int n = page->max_keyframe * page->len_geometry * GEO_INT_NUM;
    graphics_new_graph(&page->arena, &page->keyframe_graph, n);

    IGeometry *geo = graphics_page_add_geometry(page, RECT, 0);
    geo->parent_id = -1;

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

IGeometry *graphics_page_add_geometry(IPage *page, int type, int geo_id) {
    if (geo_id < 0 || geo_id >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Geometry ID %d out of range", geo_id);
        return NULL;
    }

    IGeometry *geo = geometry_create_geometry(&page->arena, type);
    geo->geo_id = geo_id;
    page->geometry[geo_id] = geo;

    return geo;
}

void graphics_free_page(IPage *page) {
    if (page == NULL) {
        return;
    }

    float arena_usage = (float) page->arena.allocd * 100 / page->arena.size;
    /*log_file(LogMessage, "Graphics", "Page %d: Num Geo %d, Arena %f \% (out of %d bytes)", */
    /*         page->temp_id, page->len_geometry, arena_usage, page->arena.size);*/
    munmap(page->arena.memory, page->arena.size);
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

        for (int attr = 0; attr < GEO_INT_NUM; attr++) {
            k_index = INDEX(geo_id, attr, frame_start, GEO_INT_NUM, page->max_keyframe);
            if (!page->keyframe_graph.exists[k_index]) {
                continue;
            }

            if (frame_start == page->max_keyframe - 1) {
                next_value = page->keyframe_graph.value[k_index];
            } else {
                next_value = graphics_keyframe_interpolate_int(
                    page->keyframe_graph.value[k_index], 
                    page->keyframe_graph.value[k_index + 1], 
                    frame_index, width
                );
            }

            geometry_set_int_attr(geo, attr, next_value);
            //log_file(LogMessage, "Graphics", "Set geo %d attr %d to %d", geo_id, attr, next_value);
        }
    }
}
