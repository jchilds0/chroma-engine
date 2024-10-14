/*
 * gr_keyframe.c 
 *
 * exports the function 
 *
 *      void            graphics_page_init_keyframe(IPage *page);
 *      unsigned int    graphics_page_add_keyframe(IPage *page)
 *      void graphics_page_set_keyframe_int(IPage *page, int frame_index, char *name, int value); 
 *      void graphics_page_set_keyframe_attr(IPage *page, int frame_index, char *name, char *value); 
 *
 *      void graphics_page_calculate_keyframes(IPage *page);
 *
 * The functions graphics_page_init_keyframe, graphics_page_add_keyframe, 
 * graphics_page_set_keyframe_int, and graphics_page_set_keyframe_attr are used 
 * to parse keyframes from the template.
 *
 * graphics_page_calculate_keyframes builds the keyframe values 
 * from the page keyframes.
 *
 */

#include "graphics.h"
#include "graphics_internal.h"
#include "geometry.h"
#include "log.h"
#include <string.h>
#include <time.h>

void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames);

static void graphics_log_keyframe(IPage *page) {
    if (!LOG_KEYFRAMES) {
        return;
    }

    // print keyframes
    log_file(LogMessage, "Graphics", "Keyframes");

    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        if (page->geometry[geo_id] == NULL) {
            continue;
        }

        log_file(LogMessage, "Graphics", "\tGeo ID %d", geo_id); 

        for (int frame_num = 0; frame_num < page->max_keyframe; frame_num++) {
            int frame_index = frame_num * page->len_geometry + geo_id;
            Node *head = &page->keyframe_graph.node_list_head[frame_index];
            Node *tail = &page->keyframe_graph.node_list_tail[frame_index];

            log_file(LogMessage, "Graphics", "\t\tFrame %d", frame_num); 

            for (Node *node = head->next; node != tail; node = node->next) {
                log_file(LogMessage, "Graphics", "\t\t\tAttr %s: %d", 
                         geometry_attr_to_char(node->attr), node->value);
            }
        }
    }
}

void graphics_page_calculate_keyframes(IPage *page) {
    int start, end;

    {
        // derived values
        IGeometry *geo;
        int width, height;

        for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
            geo = page->geometry[geo_id];

            if (geo == NULL) {
                continue;
            }

            if (geo->geo_type != TEXT) {
                continue;
            }

            width = geometry_get_int_attr(geo, GEO_WIDTH);
            graphics_graph_update_leaf(&page->keyframe_graph, geo_id, GEO_WIDTH, width);

            height = geometry_get_int_attr(geo, GEO_HEIGHT);
            graphics_graph_update_leaf(&page->keyframe_graph, geo_id, GEO_HEIGHT, height);

        }
    }

    {
        // calculate frames
        start = clock();

        graphics_graph_evaluate_dag(&page->keyframe_graph);

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Evaluated graph in %f ms", 
                     ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
        }

    }

    graphics_log_keyframe(page);

    if (LOG_KEYFRAMES) {
        log_file(LogMessage, "Graphics", "Keyframe Graph: %d nodes, %d edges", 
                 page->keyframe_graph.node_count, page->keyframe_graph.num_edges);
    }
}

static void graphics_page_root_node(IPage *page, int geo_id, int frame_num) {
    int geo_index = frame_num * page->len_geometry + geo_id;

    graphics_graph_add_leaf_node(&page->keyframe_graph, geo_index, GEO_POS_X, 0);
    graphics_graph_add_leaf_node(&page->keyframe_graph, geo_index, GEO_POS_Y, 0);
    graphics_graph_add_leaf_node(&page->keyframe_graph, geo_index, GEO_WIDTH, 1920);
    graphics_graph_add_leaf_node(&page->keyframe_graph, geo_index, GEO_HEIGHT, 1080);
}

static void graphics_geometry_relative_position(IPage *page, int geo_id, int frame_num) {
    IGeometry *geo = page->geometry[geo_id];
    int parent_id = geometry_get_int_attr(geo, GEO_PARENT);
    int parent_index = frame_num * page->len_geometry + parent_id;
    int geo_index = frame_num * page->len_geometry + geo_id; 

    {
        // x edges
        graphics_graph_add_eval_node(&page->keyframe_graph, geo_index, GEO_POS_X, EVAL_SUM_VALUE);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_POS_X, geo_index, GEO_REL_X);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_POS_X, parent_index, GEO_POS_X);

        graphics_graph_add_eval_node(&page->keyframe_graph, geo_index, GEO_X_LOWER, EVAL_SINGLE_VALUE);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_X_LOWER, geo_index, GEO_POS_X);

        graphics_graph_add_eval_node(&page->keyframe_graph, geo_index, GEO_X_UPPER, EVAL_SUM_VALUE);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_X_UPPER, geo_index, GEO_REL_X);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_X_UPPER, geo_index, GEO_WIDTH);
    }

    {
        // y edges
        graphics_graph_add_eval_node(&page->keyframe_graph, geo_index, GEO_POS_Y, EVAL_SUM_VALUE);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_POS_Y, geo_index, GEO_REL_Y);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_POS_Y, parent_index, GEO_POS_Y);

        graphics_graph_add_eval_node(&page->keyframe_graph, geo_index, GEO_Y_LOWER, EVAL_SINGLE_VALUE);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_Y_LOWER, geo_index, GEO_POS_Y);

        graphics_graph_add_eval_node(&page->keyframe_graph, geo_index, GEO_Y_UPPER, EVAL_SUM_VALUE);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_Y_UPPER, geo_index, GEO_REL_Y);
        graphics_graph_add_edge(&page->keyframe_graph, geo_index, GEO_Y_UPPER, geo_index, GEO_HEIGHT);
    }
}

static IPage *current_page = NULL;
static int geo_id = -1;

static void add_attribute(GeometryAttr attr) {
    if (current_page == NULL) {
        log_file(LogError, "Graphics", "Page not set when adding attribute");
    }

    if (geo_id == -1) {
        log_file(LogError, "Graphics", "Geo id not set when adding attribute");
    }

    Graph *g = &current_page->keyframe_graph;
    IGeometry *geo = current_page->geometry[geo_id];
    int value = geometry_get_int_attr(geo, attr);
    graphics_graph_update_leaf(g, geo_id, attr, value);

    for (int frame_num = 1; frame_num < current_page->max_keyframe; frame_num++) {
        int frame_index = frame_num * current_page->len_geometry + geo_id;
        Node *node = graphics_graph_get_node(g, frame_index, attr);

        if (node != NULL) {
            // attr is keyframed
            continue;
        }

        graphics_graph_add_eval_node(g, frame_index, attr, EVAL_SINGLE_VALUE);
        graphics_graph_add_edge(g, frame_index, attr, geo_id, attr);
    }
}

void graphics_page_default_relations(IPage *page) {
    IGeometry *geo;

    // relative positions
    for (int frame_num = 0; frame_num < page->max_keyframe; frame_num++) {
        graphics_page_root_node(page, 0, frame_num);
    }

    for (int geo_id = 1; geo_id < page->len_geometry; geo_id++) {
        geo = page->geometry[geo_id];
        if (geo == NULL) {
            continue;
        }

        for (int frame_num = 0; frame_num < page->max_keyframe; frame_num++) {
            graphics_geometry_relative_position(page, geo_id, frame_num);
        }
    }

    // default values
    current_page = page;
    for (geo_id = 1; geo_id < page->len_geometry; geo_id++) {
        geo = page->geometry[geo_id];
        if (geo == NULL) {
            continue;
        }

        geometry_graph_add_values(geo, add_attribute);
    }

    graphics_log_keyframe(page);
} 

void graphics_page_gen_frame(IPage *page, Keyframe frame) {
    if (frame.attr >= GEO_INT_NUM) {
        log_file(LogError, "Graphics", "Keyframe %d: Attr %d not implemented", frame.frame_num, frame.attr);
    }

    int frame_index = frame.frame_num * page->len_geometry + frame.geo_id; 
    int bind_index;

    switch (frame.type) {
        case USER_FRAME:
            graphics_graph_add_eval_node(&page->keyframe_graph, frame_index, frame.attr, EVAL_SINGLE_VALUE);
            graphics_graph_add_edge(&page->keyframe_graph, frame_index, frame.attr, frame.geo_id, frame.attr);
            break;

        case SET_FRAME:
            graphics_graph_add_leaf_node(&page->keyframe_graph, frame_index, frame.attr, frame.value);
            break;

        case BIND_FRAME:
            bind_index = frame.bind_frame_num * page->len_geometry + frame.bind_geo_id; // INDEX(frame.bind_geo_id, frame.bind_attr, frame.bind_frame_num, GEO_INT_NUM, page->max_keyframe);

            if (frame.bind_attr >= GEO_INT_NUM) {
                log_file(LogWarn, "Graphics", "Keyframe %d: Bind attr %d not implemented", frame.frame_num, frame.bind_attr);
                return;
            }

            graphics_graph_add_eval_node(&page->keyframe_graph, frame_index, 0, EVAL_SINGLE_VALUE);
            graphics_graph_add_edge(&page->keyframe_graph, frame_index, frame.attr, bind_index, frame.bind_attr);
            break;

        default:
            log_file(LogWarn, "Graphics", "Unknown keyframe type %d", frame.type);
    }

    if (!frame.expand) {
        return;
    }

    log_file(LogError, "Graphics", "Frame expand not implemented");

    /*
    graphics_graph_add_eval_node(&page->keyframe_graph, frame_index, geo_index, EVAL_MAX_VALUE_PAD);
    graphics_graph_add_edge(&page->keyframe_graph, frame_index, geo_index);

    for (int child_id = 0; child_id < page->len_geometry; child_id++) {
        if (page->geometry[child_id] == NULL) {
            continue;
        }

        if (frame.geo_id != geometry_get_int_attr(page->geometry[child_id], GEO_PARENT)) {
            continue;
        }

        int x_upper_index = INDEX(child_id, GEO_X_UPPER, frame.frame_num, GEO_INT_NUM, page->max_keyframe);
        int y_upper_index = INDEX(child_id, GEO_Y_UPPER, frame.frame_num, GEO_INT_NUM, page->max_keyframe);

        switch (frame.attr) {
            case GEO_WIDTH:
                graphics_graph_add_edge(&page->keyframe_graph, frame_index, x_upper_index);
                break;

            case GEO_HEIGHT: 
                graphics_graph_add_edge(&page->keyframe_graph, frame_index, y_upper_index);
                break;

            default:
                log_file(LogWarn, "Graphics", "Expand attr %d not implemented", frame.attr);
        }
    }
    */
}

/*
 * Calculate the value at time index interpolating linearly
 * from 0 to width with v_start at t = 0 and v_end at t = width
 */
int graphics_keyframe_interpolate_int(int v_start, int v_end, int index, int width) {
    if (width == 0) {
        log_file(LogWarn, "Graphics", "Interpolating over an interval of length 0");
        return v_start;
    }

    return (v_end - v_start) * index / width + v_start;
}

/* 
 * frames is a list of 0 or 1, where frames[i] == 1 
 * if values[i] is a keyframe value. Interpolate linearly 
 * between keyframe values in values.
 */
void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames) {
    int start = 0;
    int end;

    while (start < num_frames) { 
        if (!frames[start]) {
            start++;
            continue;
        }

        for (end = start + 1; end < num_frames && !frames[end]; end++);

        if (!frames[end]) {
            break;
        }

        for (int i = start + 1; i < end; i++) {
            values[i] = graphics_keyframe_interpolate_int(values[start], values[end], i, end - start);
        }

        start = end;
    }
}

