/*
 * gr_page.c 
 *
 * Functions for the IPage struct, which contains
 * the geometry of the page, how to update the 
 * geometry when it is animated on, continued or
 * animated off.
 *
 */

#include "graphics.h"
#include "graphics_internal.h"
#include "chroma-engine.h"
#include "geometry.h"
#include <stdio.h>
#include <string.h>

IPage *graphics_new_page(void ) {
    IPage *page = NEW_STRUCT(IPage);
    page->geo_head.next = &page->geo_tail;
    page->geo_head.prev = NULL;
    page->geo_tail.next = NULL;
    page->geo_tail.prev = &page->geo_head;

    page->frame_head.next = &page->frame_tail;
    page->frame_head.prev = NULL;
    page->frame_tail.next = NULL;
    page->frame_tail.prev = &page->frame_head;

    page->keyframe_graph = NULL;

    IGeometry *geo = graphics_page_add_geometry(page, RECT);
    geo->geo_id = 0;

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

IGeometry *graphics_page_add_geometry(IPage *page, int type) {
    GeometryNode *node = NEW_STRUCT(GeometryNode);
    node->geo = geometry_create_geometry(type);

    INSERT_BEFORE(node, &page->geo_tail);
    return node->geo;
}

Keyframe *graphics_page_add_keyframe(IPage *page) {
    KeyframeNode *node = NEW_STRUCT(KeyframeNode);
    Keyframe *frame = NEW_STRUCT(Keyframe);
    node->frame = frame;
    
    frame->frame_num = 0;
    frame->bind_frame_num = 0;
    frame->type = -1;
    frame->attr = -1;
    frame->bind_attr = -1;

    INSERT_BEFORE(node, &page->frame_tail);
    return node->frame;
}

void graphics_page_generate(IPage *page) {
    page->len_geometry = 0;
    page->max_keyframe = 1;

    for (GeometryNode *node = page->geo_head.next; node != &page->geo_tail; node = node->next) {
        if (node == NULL) {
            break;
        }

        if (node->geo == NULL) {
            continue;
        }

        page->len_geometry = MAX(page->len_geometry, node->geo->geo_id + 1);
    }

    for (KeyframeNode *node = page->frame_head.next; node != &page->frame_tail; node = node->next) {
        if (node == NULL) {
            break;
        }

        if (node->frame == NULL) {
            continue;
        }

        page->max_keyframe = MAX(page->max_keyframe, node->frame->frame_num + 1);
    }

    page->geometry = NEW_ARRAY(page->len_geometry, IGeometry *);

    for (int i = 0; i < page->len_geometry; i++) {
        page->geometry[i] = NULL;
    }

    int n = page->max_keyframe * page->len_geometry * GEO_INT_NUM;
    page->keyframe_graph = graphics_new_graph(n);

    // free linked lists
    GeometryNode *geo_node;
    while (page->geo_head.next != &page->geo_tail) {
        geo_node = page->geo_head.next;

        if (geo_node->geo != NULL) {
            page->geometry[geo_node->geo->geo_id] = geo_node->geo;
        }

        REMOVE_NODE(geo_node);
        free(geo_node);
    }

    KeyframeNode *key_node;
    while (page->frame_head.next != &page->frame_tail) {
        key_node = page->frame_head.next;

        if (key_node->frame != NULL) {
            graphics_page_gen_frame(page, *key_node->frame);
        }

        REMOVE_NODE(key_node);
        free(key_node);
    }
}

void graphics_free_page(IPage *page) {
    if (page == NULL) {
        return;
    }

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
            if (!page->keyframe_graph->exists[k_index]) {
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
