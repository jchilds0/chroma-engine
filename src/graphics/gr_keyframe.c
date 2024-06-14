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
#include "chroma-engine.h"
#include "geometry.h"
#include "log.h"
#include <string.h>
#include <time.h>

void graphics_page_default_values(IPage *page);
void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames);

static FRAME_TYPE keyframeToEnum(char *name) {
    if (strncmp(name, "bind-frame", KEYFRAME_TYPE_LEN) == 0) {
        return BIND_FRAME;
    } else if (strncmp(name, "user-frame", KEYFRAME_TYPE_LEN) == 0) {
        return USER_FRAME;
    } else if (strncmp(name, "set-frame", KEYFRAME_TYPE_LEN) == 0) {
        return SET_FRAME;
    }

    log_file(LogWarn, "Graphics", "Unknown keyframe type %s", name);
    return -1;
}

void graphics_keyframe_set_int(Keyframe *frame, char *name, int value) {

    if (strncmp(name, "frame_num", KEYFRAME_TYPE_LEN) == 0) {

        frame->frame_num = value;

    } else if (strncmp(name, "frame_geo", KEYFRAME_TYPE_LEN) == 0) {

        frame->geo_id = value;

    } else if (strncmp(name, "value", KEYFRAME_TYPE_LEN) == 0) {

        frame->value = value;

    } else if (strncmp(name, "bind_frame", KEYFRAME_TYPE_LEN) == 0) {

        frame->bind_frame_num = value;

    } else if (strncmp(name, "bind_geo", KEYFRAME_TYPE_LEN) == 0) {

        frame->bind_geo_id = value;

    } else {

        log_file(LogWarn, "Graphics", "Keyframe attr %s is not an int attr", name);

    }
}

void graphics_keyframe_set_attr(Keyframe *frame, char *name, char *value) {
    if (strncmp(name, "frame_type", KEYFRAME_TYPE_LEN) == 0) {

        frame->type = keyframeToEnum(value); 

    } else if (strncmp(name, "frame_attr", KEYFRAME_TYPE_LEN) == 0) {

        frame->attr = geometry_char_to_attr(value); 

    } else if (strncmp(name, "expand", KEYFRAME_TYPE_LEN) == 0) {

        if (strncmp(value, "true", KEYFRAME_TYPE_LEN) == 0) {
            frame->expand = 1;
        } else {
            frame->expand = 0;
        }

    } else if (strncmp(name, "user_frame", KEYFRAME_TYPE_LEN) == 0) {

        frame->type = USER_FRAME;

    } else if (strncmp(name, "bind_attr", KEYFRAME_TYPE_LEN) == 0) {

        frame->bind_attr = geometry_char_to_attr(value); 

    } else {

        log_file(LogWarn, "Graphics", "Keyframe attr %s is not a string attr", name);

    }
}

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

        log_file(LogMessage, "Graphics", "\tGeo ID %d: ", geo_id); 

        for (int attr = 0; attr < GEO_NUM; attr++) {
            if (!geometry_is_int_attr(attr)) {
                continue;
            }

            log_file(LogMessage, "Graphics", "\t\tAttr %d: ", attr);

            for (int frame_index = 0; frame_index < page->max_keyframe; frame_index++) {
                int i = INDEX(geo_id, attr, frame_index, GEO_NUM, page->max_keyframe);

                log_file(LogMessage, "Graphics", "\t\t\tFrame %d: %d", frame_index, 
                         page->keyframe_graph->value[i]); 
            }
        }
    }
}

void graphics_page_calculate_keyframes(IPage *page) {
    int start, end;

    {
        // derived values
        IGeometry *geo;
        int width, height, frame_index;

        for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
            geo = page->geometry[geo_id];

            if (geo == NULL) {
                continue;
            }

            if (geo->geo_type != TEXT) {
                continue;
            }

            width = geometry_get_int_attr(geo, GEO_WIDTH);
            frame_index = INDEX(geo_id, GEO_WIDTH, 0, GEO_NUM, page->max_keyframe);
            graphics_graph_add_leaf_node(page->keyframe_graph, frame_index, width);

            height = geometry_get_int_attr(geo, GEO_HEIGHT);
            frame_index = INDEX(geo_id, GEO_HEIGHT, 0, GEO_NUM, page->max_keyframe);
            graphics_graph_add_leaf_node(page->keyframe_graph, frame_index, height);

        }
    }

    {
        // calculate frames
        start = clock();

        if (!graphics_graph_is_dag(page->keyframe_graph)) {
            log_file(LogError, "Graphics", "Page %d keyframes are not in a dag", page->temp_id);
        }

        graphics_graph_evaluate_dag(page->keyframe_graph);

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Evaluated graph in %f ms", 
                     ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
        }

    }

    graphics_log_keyframe(page);
}

static int single_value(Node node) {
    int value_count = 0;
    int value = 0;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }

        value_count++;
        value = node.values[i];
    }

    if (value_count != 1) {
        log_file(LogError, "Graphics", "Node %d has %d values, expected 1", node.node_index, value_count);
    }

    return value;
}

static int min_value(Node node) {
    int value = INT_MAX;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }
        
        value = MIN(value, node.values[i]);
    }

    if (value == INT_MAX) {
        log_file(LogError, "Graphics", "Node %d missing values", node.node_index);
    }

    return value;
}

static int max_value(Node node) {
    int value = INT_MIN;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }
        
        value = MAX(value, node.values[i]);
    }

    if (value == INT_MIN) {
        log_file(LogError, "Graphics", "Node %d missing values", node.node_index);
    }

    return value;
}

static int max_value_plus_pad(Node node) {
    int value = INT_MIN;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }

        if (i == node.pad_index) {
            continue;
        }
        
        value = MAX(value, node.values[i]);
    }

    if (value == INT_MIN) {
        log_file(LogError, "Graphics", "Node %d missing values", node.node_index);
    }

    if (!node.have_value[node.pad_index]) {
        log_file(LogError, "Graphics", "Node %d missing pad %d", node.node_index, node.pad_index);
    }

    return value + node.values[node.pad_index];
}

static int sum_value(Node node) {
    int value = 0; 

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }

        value += node.values[i];
    }

    return value;
}

static void graphics_page_root_node(IPage *page, int geo_id, int frame_num) {
    int pos_x_index = INDEX(geo_id, GEO_POS_X, frame_num, GEO_NUM, page->max_keyframe);
    int pos_y_index = INDEX(geo_id, GEO_POS_Y, frame_num, GEO_NUM, page->max_keyframe);
    int width_index = INDEX(geo_id, GEO_WIDTH, frame_num, GEO_NUM, page->max_keyframe);
    int height_index = INDEX(geo_id, GEO_HEIGHT, frame_num, GEO_NUM, page->max_keyframe);

    graphics_graph_add_eval_node(page->keyframe_graph, pos_x_index, 0, NULL);
    graphics_graph_add_eval_node(page->keyframe_graph, pos_y_index, 0, NULL);

    graphics_graph_add_eval_node(page->keyframe_graph, width_index, 1920, NULL);
    graphics_graph_add_eval_node(page->keyframe_graph, height_index, 1080, NULL);
}

static void graphics_geometry_relative_position(IPage *page, int geo_id, int frame_num) {
    IGeometry *geo = page->geometry[geo_id];
    int parent_id = geometry_get_int_attr(geo, GEO_PARENT);

    {
        // x edges
        int pos_x_index     = INDEX(geo_id, GEO_POS_X, frame_num, GEO_NUM, page->max_keyframe);
        int rel_x_index     = INDEX(geo_id, GEO_REL_X, frame_num, GEO_NUM, page->max_keyframe);
        int width_index     = INDEX(geo_id, GEO_WIDTH, frame_num, GEO_NUM, page->max_keyframe);
        int lower_x_index   = INDEX(geo_id, GEO_X_LOWER, frame_num, GEO_NUM, page->max_keyframe);
        int upper_x_index   = INDEX(geo_id, GEO_X_UPPER, frame_num, GEO_NUM, page->max_keyframe);
        int parent_x_index  = INDEX(parent_id, GEO_POS_X, frame_num, GEO_NUM, page->max_keyframe);

        graphics_graph_add_eval_node(page->keyframe_graph, pos_x_index, 0, sum_value);
        graphics_graph_add_edge(page->keyframe_graph, pos_x_index, rel_x_index);
        graphics_graph_add_edge(page->keyframe_graph, pos_x_index, parent_x_index);

        graphics_graph_add_eval_node(page->keyframe_graph, lower_x_index, 0, single_value);
        graphics_graph_add_edge(page->keyframe_graph, lower_x_index, pos_x_index);

        graphics_graph_add_eval_node(page->keyframe_graph, upper_x_index, 0, sum_value);
        graphics_graph_add_edge(page->keyframe_graph, upper_x_index, rel_x_index);
        graphics_graph_add_edge(page->keyframe_graph, upper_x_index, width_index);
    }

    {
        // y edges
        int pos_y_index     = INDEX(geo_id, GEO_POS_Y, frame_num, GEO_NUM, page->max_keyframe);
        int rel_y_index     = INDEX(geo_id, GEO_REL_Y, frame_num, GEO_NUM, page->max_keyframe);
        int height_index     = INDEX(geo_id, GEO_HEIGHT, frame_num, GEO_NUM, page->max_keyframe);
        int lower_y_index   = INDEX(geo_id, GEO_Y_LOWER, frame_num, GEO_NUM, page->max_keyframe);
        int upper_y_index   = INDEX(geo_id, GEO_Y_UPPER, frame_num, GEO_NUM, page->max_keyframe);
        int parent_y_index  = INDEX(parent_id, GEO_POS_Y, frame_num, GEO_NUM, page->max_keyframe);

        graphics_graph_add_eval_node(page->keyframe_graph, pos_y_index, 0, sum_value);
        graphics_graph_add_edge(page->keyframe_graph, pos_y_index, rel_y_index);
        graphics_graph_add_edge(page->keyframe_graph, pos_y_index, parent_y_index);

        graphics_graph_add_eval_node(page->keyframe_graph, lower_y_index, 0, single_value);
        graphics_graph_add_edge(page->keyframe_graph, lower_y_index, pos_y_index);

        graphics_graph_add_eval_node(page->keyframe_graph, upper_y_index, 0, sum_value);
        graphics_graph_add_edge(page->keyframe_graph, upper_y_index, rel_y_index);
        graphics_graph_add_edge(page->keyframe_graph, upper_y_index, height_index);
    }
}

static void graphics_geometry_default_values(IPage *page, int geo_id, int attr) {
    for (int frame_num = 1; frame_num < page->max_keyframe; frame_num++) {
        int base_index = INDEX(geo_id, attr, 0, GEO_NUM, page->max_keyframe);
        int frame_index = INDEX(geo_id, attr, frame_num, GEO_NUM, page->max_keyframe);

        if (page->keyframe_graph->exists[frame_index]) {
            continue;
        }

        graphics_graph_add_eval_node(page->keyframe_graph, frame_index, 0, single_value);
        graphics_graph_add_edge(page->keyframe_graph, frame_index, base_index);
    }
}

void graphics_page_default_relations(IPage *page) {
    // relative positions
    for (int frame_num = 0; frame_num < page->max_keyframe; frame_num++) {
        graphics_page_root_node(page, 0, frame_num);
    }

    for (int geo_id = 1; geo_id < page->len_geometry; geo_id++) {
        if (page->geometry[geo_id] == NULL) {
            continue;
        }

        for (int frame_num = 0; frame_num < page->max_keyframe; frame_num++) {
            graphics_geometry_relative_position(page, geo_id, frame_num);
        }
    }

    // default values
    graphics_page_default_values(page);

    for (int geo_id = 1; geo_id < page->len_geometry; geo_id++) {
        for (int attr = 0; attr < GEO_NUM; attr++) {
            int base_index = INDEX(geo_id, attr, 0, GEO_NUM, page->max_keyframe);

            if (!page->keyframe_graph->exists[base_index]) {
                continue;
            }

            graphics_geometry_default_values(page, geo_id, attr);
        }
    }

    graphics_log_keyframe(page);
} 

static IPage *page = NULL;
static int geo_id = -1;

static void add_attribute(int attr) {
    if (page == NULL) {
        log_file(LogError, "Graphics", "Page not set when adding attribute");
    }

    if (geo_id == -1) {
        log_file(LogError, "Graphics", "Geo id not set when adding attribute");
    }

    IGeometry *geo = page->geometry[geo_id];
    int frame_index = INDEX(geo_id, attr, 0, GEO_NUM, page->max_keyframe);
    int value = geometry_get_int_attr(geo, attr);

    graphics_graph_add_leaf_node(page->keyframe_graph, frame_index, value);
}


void graphics_page_default_values(IPage *cur_page) {
    page = cur_page;
    for (geo_id = 1; geo_id < cur_page->len_geometry; geo_id++) {
        IGeometry *geo = cur_page->geometry[geo_id];
        if (geo == NULL) {
            continue;
        }

        geometry_graph_add_values(geo, add_attribute);
    }
}

void graphics_page_add_keyframe(IPage *page, Keyframe frame) {
    int frame_index = INDEX(frame.geo_id, frame.attr, frame.frame_num, GEO_NUM, page->max_keyframe);
    int geo_index = INDEX(frame.geo_id, frame.attr, 0, GEO_NUM, page->max_keyframe);
    int bind_index;

    switch (frame.type) {
        case USER_FRAME:
            graphics_graph_add_eval_node(page->keyframe_graph, frame_index, 0, single_value);
            graphics_graph_add_edge(page->keyframe_graph, frame_index, geo_index);
            break;

        case SET_FRAME:
            graphics_graph_add_leaf_node(page->keyframe_graph, frame_index, frame.value);
            break;

        case BIND_FRAME:
            bind_index = INDEX(
                frame.bind_geo_id, frame.bind_attr, frame.bind_frame_num, 
                GEO_NUM, page->max_keyframe
            );

            graphics_graph_add_eval_node(page->keyframe_graph, frame_index, 0, single_value);
            graphics_graph_add_edge(page->keyframe_graph, frame_index, bind_index);
            break;

        default:
            log_file(LogWarn, "Graphics", "Unknown keyframe type %d", frame.type);
    }

    if (LOG_KEYFRAMES) {
        log_file(LogMessage, "Graphics", "Keyframe %d", frame.frame_num);
        log_file(
            LogMessage, "Graphics", 
            "\tInitially set geo %d, attr %d to %d", 
            frame.geo_id, frame.attr, page->keyframe_graph->value[frame_index]
        );
    }

    if (!frame.expand) {
        return;
    }

    graphics_graph_add_eval_node(page->keyframe_graph, frame_index, geo_index, max_value_plus_pad);
    graphics_graph_add_edge(page->keyframe_graph, frame_index, geo_index);

    for (int child_id = 0; child_id < page->len_geometry; child_id++) {
        if (page->geometry[child_id] == NULL) {
            continue;
        }

        if (frame.geo_id != geometry_get_int_attr(page->geometry[child_id], GEO_PARENT)) {
            continue;
        }

        int x_upper_index = INDEX(child_id, GEO_X_UPPER, frame.frame_num, GEO_NUM, page->max_keyframe);
        int y_upper_index = INDEX(child_id, GEO_Y_UPPER, frame.frame_num, GEO_NUM, page->max_keyframe);

        switch (frame.attr) {
            case GEO_WIDTH:
                graphics_graph_add_edge(page->keyframe_graph, frame_index, x_upper_index);
                break;

            case GEO_HEIGHT: 
                graphics_graph_add_edge(page->keyframe_graph, frame_index, y_upper_index);
                break;

            default:
                log_file(LogWarn, "Graphics", "Expand attr %d not implemented", frame.attr);
        }
    }
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

