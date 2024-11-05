/*
 * gr_page.c 
 *
 * Functions for the IPage struct, which contains
 * the geometry of the page, how to update the 
 * geometry when it is animated on, continued or
 * animated off.
 *
 */

#include "arena.h"
#include "graphics_internal.h"

void graphics_page_init_arena(IPage *page) {
    pthread_mutex_lock(&page->lock);
    ARENA_INIT(&page->arena, MAX_PAGE_SIZE);
    pthread_mutex_unlock(&page->lock);
}

void graphics_init_page(IPage *page, int temp_id, int num_geo, int max_keyframe) {
    log_assert(page != NULL, "Graphics", "Page init requires a page");

    pthread_mutex_lock(&page->lock);
    log_assert(page->arena.memory != NULL, "Graphics", "Page init requires arena to be allocated");

    page->temp_id = temp_id;
    page->len_geometry = num_geo;
    page->max_keyframe = max_keyframe;
    page->geometry = ARENA_ARRAY(&page->arena, num_geo, IGeometry *);

    int n = page->max_keyframe * page->len_geometry;
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
    pthread_mutex_unlock(&page->lock);
}

IGeometry *graphics_page_add_geometry(IPage *page, int type, int geo_id) {
    if (geo_id < 0 || geo_id >= page->len_geometry) {
        log_file(LogWarn, "Graphics", "Geometry ID %d out of range", geo_id);
        pthread_mutex_unlock(&page->lock);
        return NULL;
    }

    IGeometry *geo = geometry_create_geometry(&page->arena, type);
    geo->geo_id = geo_id;
    page->geometry[geo_id] = geo;

    return geo;
}

void graphics_page_clear(IPage *page) {
    if (page == NULL) {
        return;
    }

    double arena_usage = (double) page->arena.allocd * 100 / page->arena.size;
    uint64_t usage_size = page->arena.allocd / MEGABYTES((uint64_t)1);
    uint64_t arena_size = page->arena.size / MEGABYTES((uint64_t)1);
    uint64_t graph_size = graphics_graph_size(&page->keyframe_graph) / MEGABYTES((uint64_t)1);

    log_file(LogMessage, "Graphics", "Page %d", page->temp_id); 
    log_file(LogMessage, "Graphics", "\tNum Geo %d", page->len_geometry);
    log_file(LogMessage, "Graphics", "\tArena %f %% (%lu out of %lu MB)", arena_usage, usage_size, arena_size);
    log_file(LogMessage, "Graphics", "\tGraph %lu nodes, %lu MB", page->keyframe_graph.num_nodes, graph_size);

    page->arena.allocd = 0;
    page->len_geometry = 0;
}

int graphics_page_free_page(IPage *page) {
    if (page == NULL) {
        return 0;
    }

    return munmap(page->arena.memory, page->arena.allocd);
}

void graphics_page_interpolate_geometry(IPage *page, int index, int width) {
    IGeometry *geo;
    Node *head, *tail;
    int k_index, k1_index;
    int frame_start = index / width;
    int frame_index = index % width;
    double next_value;

    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        geo = page->geometry[geo_id];
        if (geo == NULL) {
            continue;
        }

        k_index = frame_start * page->len_geometry + geo_id;
        k1_index = (frame_start + 1) * page->len_geometry + geo_id;
        head = &page->keyframe_graph.node_list_head[k_index];
        tail = &page->keyframe_graph.node_list_tail[k_index];

        for (Node *node = head->next; node != tail; node = node->next) {
            if (!node->evaluated) {
                continue;
            }

            if (frame_start == page->max_keyframe - 1) {
                geometry_set_float_attr(geo, node->attr, node->value);
                continue;
            } 

            Node *next_node = graphics_graph_get_node(&page->keyframe_graph, k1_index, node->attr);
            if (next_node == NULL) {
                next_node = node;
            }

            next_value = graphics_keyframe_interpolate(
                node->value, next_node->value, frame_index, width
            );

            geometry_set_float_attr(geo, node->attr, next_value);
        }
    }
}
