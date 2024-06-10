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

#include "graphics_internal.h"
#include "chroma-engine.h"
#include "geometry.h"
#include "log.h"
#include <string.h>
#include <time.h>

void graphics_page_add_edges(IPage *page, Graph *g);
void graphics_keyframe_store_value(IPage *page, Graph *g, Keyframe *frame);
void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames);

static int keyframeToEnum(char *name) {
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

/*
 * Next available keyframe index
 */
unsigned int graphics_page_add_keyframe(IPage *page) {
    if (page->num_keyframe == page->len_keyframe) {
        log_file(LogWarn, "Graphics", "Out of keyframe memory");
        return -1;
    }

    page->num_keyframe++;
    return page->num_keyframe - 1;
}

/*
 * Allocate page keyframe arrays
 */
void graphics_page_init_keyframe(IPage *page) {
    page->max_keyframe = 0;

    for (int i = 0; i < page->len_keyframe; i++) {
        page->max_keyframe = MAX(page->max_keyframe, page->keyframe[i].frame_num);
    }

    page->max_keyframe++;

    free(page->k_value);

    int n = page->max_keyframe * page->len_geometry * GEO_NUM;
    page->k_value = NEW_ARRAY(n, int);

    for (int i = 0; i < n; i++) {
        page->k_value[i] = -1;
    }
}

void graphics_page_set_keyframe_int(IPage *page, int keyframe_index, char *name, int value) {
    if (keyframe_index < 0 || keyframe_index >= page->num_keyframe) {
        log_file(LogError, "Graphics", "Keyframe index %d out of range", keyframe_index);
        return;
    }

    Keyframe *frame = &page->keyframe[keyframe_index];

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

void graphics_page_set_keyframe_attr(IPage *page, int keyframe_index, char *name, char *value) {
    if (keyframe_index < 0 || keyframe_index >= page->num_keyframe) {
        log_file(LogError, "Graphics", "Keyframe index %d out of range", keyframe_index);
        return;
    }

    Keyframe *frame = &page->keyframe[keyframe_index];

    if (strncmp(name, "frame_type", KEYFRAME_TYPE_LEN) == 0) {
        frame->type = keyframeToEnum(value); 
    } else if (strncmp(name, "frame_attr", KEYFRAME_TYPE_LEN) == 0) {
        frame->attr = geometry_char_to_attr(value); 
    } else if (strncmp(name, "mask", KEYFRAME_TYPE_LEN) == 0) {
        if (strncmp(value, "true", KEYFRAME_TYPE_LEN) == 0) {
            frame->mask = 1;
        } else {
            frame->mask = 0;
        }
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

/*
 * Build a 3D array of the geometry attribute values per 
 * geometry and per keyframe.
 */
void graphics_page_calculate_keyframes(IPage *page) {
    int start, end;
    int n = page->max_keyframe * page->len_geometry * GEO_NUM;

    Graph *g = graphics_new_graph(n);

    {
        // store geometry relations
        start = clock();

        graphics_page_add_edges(page, g);

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Parsed Keyframes in %f ms", 
                ((double) (end - start) * 1000) / CLOCKS_PER_SEC
            );
        }

    }

    {
        // store keyframe value
        start = clock();
        for (int i = 0; i < page->len_keyframe; i++) {
            Keyframe *frame = &page->keyframe[i];

            graphics_keyframe_store_value(page, g, frame);
        }

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Parsed Keyframes in %f ms", 
                ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
        }

    }

    {
        // calculate frames
        start = clock();

        if (!graphics_graph_is_dag(g)) {
            log_file(LogError, "Graphics", "Page %d keyframes are not in a dag", page->temp_id);
        }

        graphics_graph_evaluate_dag(g);

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Evaluated graph in %f ms", 
                     ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
        }

    }

    for (int i = 0; i < n; i++) {
        page->k_value[i] = g->value[i];
    }

    graphics_graph_free_graph(g);

    if (!LOG_KEYFRAMES) {
        return;
    }

    // print keyframes
    log_file(LogMessage, "Graphics", "Final Keyframes");

    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        log_file(LogMessage, "Graphics", "\tGeo ID %d: ", geo_id); 

        for (int attr = 0; attr < GEO_NUM; attr++) {
            if (!page->attr_keyframe[geo_id * GEO_NUM + attr]) {
                continue;
            }

            log_file(LogMessage, "Graphics", "\t\tAttr %d: ", attr);

            for (int frame_index = 0; frame_index < page->max_keyframe; frame_index++) {
                int i = INDEX(geo_id, attr, frame_index, GEO_NUM, page->max_keyframe);

                log_file(LogMessage, "Graphics", "\t\t\tFrame %d: %d", frame_index, page->k_value[i]); 
            }
        }
    }
}

static int single_value(int *values, unsigned char *have_value, int num_values) {
    int value_count = 0;
    int value = 0;

    for (int i = 0; i < num_values; i++) {
        if (!have_value[i]) {
            continue;
        }

        value_count++;
        value = values[i];
    }

    if (value_count != 1) {
        log_file(LogError, "Graphics", "Incorrect have_value array, count %d", value_count);
    }

    return value;
}

static int min_value(int *values, unsigned char *have_value, int num_values) {
    int value = INT_MAX;

    for (int i = 0; i < num_values; i++) {
        if (!have_value[i]) {
            continue;
        }
        
        value = MIN(value, values[i]);
    }

    return value;
}

static int max_value(int *values, unsigned char *have_value, int num_values) {
    int value = INT_MIN;

    for (int i = 0; i < num_values; i++) {
        if (!have_value[i]) {
            continue;
        }
        
        value = MAX(value, values[i]);
    }

    return value;
}

static int sum_value(int *values, unsigned char *have_value, int num_values) {
    int value = 0; 

    for (int i = 0; i < num_values; i++) {
        if (!have_value[i]) {
            continue;
        }

        value += values[i];
    }

    return value;
}

static void graphics_geometry_add_edges(IGeometry *geo, Graph *g, int geo_id, int frame_num, int num_frames) {
    int parent_id = geometry_get_int_attr(geo, GEO_PARENT);

    {
        // x edges
        int pos_x_index     = INDEX(geo_id, GEO_POS_X, frame_num, GEO_NUM, num_frames);
        int rel_x_index     = INDEX(geo_id, GEO_REL_X, frame_num, GEO_NUM, num_frames);
        int width_index     = INDEX(geo_id, GEO_WIDTH, frame_num, GEO_NUM, num_frames);
        int lower_x_index   = INDEX(geo_id, GEO_X_LOWER, frame_num, GEO_NUM, num_frames);
        int upper_x_index   = INDEX(geo_id, GEO_X_UPPER, frame_num, GEO_NUM, num_frames);
        int parent_x_index  = INDEX(parent_id, GEO_POS_X, frame_num, GEO_NUM, num_frames);

        int rel_x = geometry_get_int_attr(geo, GEO_REL_X);
        int width = geometry_get_int_attr(geo, GEO_WIDTH);

        graphics_graph_add_node(g, rel_x_index, rel_x, NULL);
        graphics_graph_add_node(g, width_index, width, NULL);

        graphics_graph_add_node(g, pos_x_index, 0, sum_value);
        graphics_graph_add_edge(g, pos_x_index, rel_x_index);
        graphics_graph_add_edge(g, pos_x_index, parent_x_index);

        graphics_graph_add_node(g, lower_x_index, 0, single_value);
        graphics_graph_add_edge(g, lower_x_index, pos_x_index);

        graphics_graph_add_node(g, upper_x_index, 0, sum_value);
        graphics_graph_add_edge(g, upper_x_index, rel_x_index);
        graphics_graph_add_edge(g, upper_x_index, width_index);
    }

    {
        // y edges
        int pos_y_index     = INDEX(geo_id, GEO_POS_Y, frame_num, GEO_NUM, num_frames);
        int rel_y_index     = INDEX(geo_id, GEO_REL_Y, frame_num, GEO_NUM, num_frames);
        int height_index     = INDEX(geo_id, GEO_HEIGHT, frame_num, GEO_NUM, num_frames);
        int lower_y_index   = INDEX(geo_id, GEO_Y_LOWER, frame_num, GEO_NUM, num_frames);
        int upper_y_index   = INDEX(geo_id, GEO_Y_UPPER, frame_num, GEO_NUM, num_frames);
        int parent_y_index  = INDEX(parent_id, GEO_POS_Y, frame_num, GEO_NUM, num_frames);

        int rel_y = geometry_get_int_attr(geo, GEO_REL_Y);
        int height = geometry_get_int_attr(geo, GEO_HEIGHT);

        graphics_graph_add_node(g, rel_y_index, rel_y, NULL);
        graphics_graph_add_node(g, height_index, height, NULL);

        graphics_graph_add_node(g, pos_y_index, 0, sum_value);
        graphics_graph_add_edge(g, pos_y_index, rel_y_index);
        graphics_graph_add_edge(g, pos_y_index, parent_y_index);

        graphics_graph_add_node(g, lower_y_index, 0, single_value);
        graphics_graph_add_edge(g, lower_y_index, pos_y_index);

        graphics_graph_add_node(g, upper_y_index, 0, sum_value);
        graphics_graph_add_edge(g, upper_y_index, rel_y_index);
        graphics_graph_add_edge(g, upper_y_index, height_index);
    }
}

void graphics_page_add_edges(IPage *page, Graph *g) {
    for (int frame_num = 0; frame_num < page->max_keyframe; frame_num++) {
        for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
            if (page->geometry[geo_id] == NULL) {
                continue;
            }

            if (geo_id == geometry_get_int_attr(page->geometry[geo_id], GEO_PARENT)) {
                continue;
            }

            graphics_geometry_add_edges(page->geometry[geo_id], g, geo_id, frame_num, page->max_keyframe);
        }
    }
} 

void graphics_keyframe_store_value(IPage *page, Graph *g, Keyframe *frame) {
    IGeometry *geo = page->geometry[frame->geo_id];
    int frame_index = INDEX(frame->geo_id, frame->attr, frame->frame_num, GEO_NUM, page->max_keyframe);
    int bind_index;

    page->attr_keyframe[frame->geo_id * GEO_NUM + frame->attr] = 1;

    switch (frame->type) {
        case USER_FRAME:
            graphics_graph_add_node(g, frame_index, geometry_get_int_attr(geo, frame->attr), NULL);
            break;

        case SET_FRAME:
            graphics_graph_add_node(g, frame_index, frame->value, NULL);
            break;

        case BIND_FRAME:
            bind_index = INDEX(
                frame->bind_geo_id, frame->bind_attr, frame->bind_frame_num, 
                GEO_NUM, page->max_keyframe
            );

            graphics_graph_add_node(g, frame_index, 0, single_value);
            graphics_graph_add_edge(g, frame_index, bind_index);
            break;

        default:
            log_file(LogWarn, "Graphics", "Unknown keyframe type %d", frame->type);
    }

    if (LOG_KEYFRAMES) {
        log_file(LogMessage, "Graphics", "Keyframe %d", frame->frame_num);
        log_file(
            LogMessage, "Graphics", 
            "\tInitially set geo %d, attr %d to %d", 
            frame->geo_id, frame->attr, page->k_value[frame_index]
        );
    }

    if (!frame->expand) {
        return;
    }

    graphics_graph_add_node(g, frame_index, g->value[frame_index], max_value);
    graphics_graph_add_edge(g, frame_index, frame_index);
    int x_upper_index, y_upper_index;

    for (int child_id = 0; child_id < page->len_geometry; child_id++) {
        if (page->geometry[child_id] == NULL) {
            continue;
        }

        if (frame->geo_id != geometry_get_int_attr(page->geometry[child_id], GEO_PARENT)) {
            continue;
        }

        x_upper_index = INDEX(child_id, GEO_X_UPPER, frame->frame_num, GEO_NUM, page->max_keyframe);
        y_upper_index = INDEX(child_id, GEO_Y_UPPER, frame->frame_num, GEO_NUM, page->max_keyframe);

        switch (frame->attr) {
            case GEO_WIDTH:
                graphics_graph_add_edge(g, frame_index, x_upper_index);
                break;

            case GEO_HEIGHT: 
                graphics_graph_add_edge(g, frame_index, y_upper_index);
                break;

            default:
                log_file(LogWarn, "Graphics", "Expand attr %d not implemented", frame->attr);
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

