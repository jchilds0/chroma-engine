/*
 * gr_keyframe.c 
 *
 *
 */

#include "graphics_internal.h"
#include "chroma-engine.h"
#include "geometry.h"
#include "log.h"
#include <string.h>

FRAME_ATTR graphics_keyframe_attr(char *attr);
void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames);

unsigned int graphics_page_add_keyframe(IPage *page) {
    if (page->num_keyframe == page->len_keyframe) {
        log_file(LogWarn, "Graphics", "Out of keyframe memory");
        return -1;
    }

    page->num_keyframe++;
    return page->num_keyframe - 1;
}

void graphics_page_init_keyframe(IPage *page) {
    page->max_keyframe = 0;

    for (int i = 0; i < page->len_keyframe; i++) {
        page->max_keyframe = MAX(page->max_keyframe, page->keyframe[i].frame_num);
    }

    free(page->k_value);
    page->k_value = NEW_ARRAY(page->max_keyframe * page->len_geometry * NUM_ATTR, int);
    for (int i = 0; i < page->max_keyframe * page->len_geometry * NUM_ATTR; i++) {
        page->k_value[i] = -1;
    }
}

void graphics_page_set_keyframe_int(IPage *page, int keyframe_index, char *name, int value) {
    if (keyframe_index < 0 || keyframe_index >= page->num_keyframe) {
        log_file(LogError, "Graphics", "Keyframe index %d out of range", keyframe_index);
        return;
    }

    Keyframe *frame = &page->keyframe[keyframe_index];

    if (strcmp(name, "frame_num") == 0) {
        frame->frame_num = value;
    } else if (strcmp(name, "frame_geo") == 0) {
        frame->geo_id = value;
    } else if (strcmp(name, "value") == 0) {
        frame->type = SET_VALUE;
        frame->value = value;
    } else if (strcmp(name, "bind_frame") == 0) {
        frame->type = BIND_VALUE;
        frame->bind_frame_num = value;
    } else if (strcmp(name, "bind_geo") == 0) {
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

    if (strcmp(name, "frame_attr") == 0) {
        memcpy(frame->attr, value, GEO_BUF_SIZE);
    } else if (strcmp(name, "mask") == 0) {
        if (strcmp(value, "true") == 0) {
            frame->mask = 1;
        } else {
            frame->mask = 0;
        }
    } else if (strcmp(name, "expand") == 0) {
        if (strcmp(value, "true") == 0) {
            frame->expand = 1;
        } else {
            frame->expand = 0;
        }
    } else if (strcmp(name, "user_frame") == 0) {
        frame->type = USER_VALUE;
    } else if (strcmp(name, "bind_attr") == 0) {
        memcpy(frame->bind_attr, value, GEO_BUF_SIZE);
    } else {
        log_file(LogWarn, "Graphics", "Keyframe attr %s is not a string attr", name);
    }

}

FRAME_ATTR graphics_keyframe_attr_int(char *attr) {
    if (strncmp(attr, "pos_x", 5) == 0) {
        return POS_X;
    } else if (strncmp(attr, "pos_y", 5) == 0) {
        return POS_Y;
    } else if (strncmp(attr, "rel_x", 5) == 0) {
        return REL_X;
    } else if (strncmp(attr, "rel_y", 5) == 0) {
        return REL_Y;
    } else if (strncmp(attr, "parent", 6) == 0) {
        return PARENT;
    } else if (strncmp(attr, "width", 5) == 0) {
        return WIDTH;
    } else if (strncmp(attr, "height", 6) == 0) {
        return HEIGHT;
    } else if (strncmp(attr, "rounding", 8) == 0) {
        return ROUNDING;
    } else if (strncmp(attr, "inner_radius", 12) == 0) {
        return INNER_RADIUS;
    } else if (strncmp(attr, "outer_radius", 12) == 0) {
        return OUTER_RADIUS;
    } else if (strncmp(attr, "start_angle", 11) == 0) {
        return START_ANGLE;
    } else if (strncmp(attr, "end_angle", 9) == 0) {
        return END_ANGLE;
    } else {
        log_file(LogWarn, "Geometry", "Unknown geometry attr (%s)", attr);
        return -1;
    }


}

static unsigned char graphics_keyframe_exists(unsigned char *frames, int num_frames) {
    unsigned char found_keyframe = 0;

    for (int i = 0; i < num_frames; i++) {
        if (frames[i] == 1) {
            found_keyframe = 1;
        }
    }

    return found_keyframe;
}

void graphics_page_calculate_keyframes(IPage *page) {
    IGeometry *geo;
    int frame_index;

    unsigned char *have_keyframe = NEW_ARRAY(page->max_keyframe * page->len_geometry * NUM_ATTR, unsigned char);

    for (int i = 0; i < NUM_ATTR * page->len_geometry * page->max_keyframe; i++) {
        have_keyframe[i] = 0;
    }

    // store keyframe value
    for (int i = 0; i < page->len_keyframe; i++) {
        Keyframe *frame = &page->keyframe[i];
        FRAME_ATTR attr = graphics_keyframe_attr_int(frame->attr);

        geo = page->geometry[frame->geo_id];
        frame_index = frame->geo_id * (page->max_keyframe * NUM_ATTR) 
            + attr * page->max_keyframe + frame->frame_num;
        have_keyframe[frame_index] = 1;
        page->attr_keyframe[frame->geo_id * NUM_ATTR + attr] = 1;

        switch (frame->type) {
            case USER_VALUE:
                page->k_value[frame_index] = geometry_get_int_attr(geo, frame->attr);
                break;
            case SET_VALUE:
                page->k_value[frame_index] = frame->value;
                break;
            case BIND_VALUE:
                log_file(LogError, "Graphics", "Bind value not implemented");
                break;
            default:
                log_file(LogWarn, "Graphics", "Unknown keyframe type %d", frame->type);
        }

        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Keyframe %d", frame->frame_num);
            log_file(LogMessage, "Graphics", "\tInitially set geo %d, attr %s to %d", frame->geo_id, frame->attr, page->k_value[frame_index]);
        }

        if (frame->expand) {
            for (int geo_id = 0; geo_id < page->num_geometry; geo_id++) {
                if (page->geometry[geo_id]->parent != frame->geo_id) {
                    continue;
                }

                int current_val = page->k_value[frame_index];
                int parent_padding = geometry_get_int_attr(geo, frame->attr);
                int child_val = geometry_get_int_attr(page->geometry[geo_id], frame->attr);
                
                if (strcmp(frame->attr, "width") == 0) {
                    child_val += geometry_get_int_attr(page->geometry[geo_id], "rel_x");
                } else if (strcmp(frame->attr, "height") == 0) {
                    child_val += geometry_get_int_attr(page->geometry[geo_id], "rel_y");
                } else {
                    log_file(LogWarn, "Graphics", "Expand %s not implemented", frame->attr);
                    continue;
                }

                if (child_val + parent_padding > current_val && LOG_KEYFRAMES) {
                    log_file(LogMessage, "Graphics", "\tUpdated to expand for child %d to %d", geo_id, child_val + parent_padding);
                }
                page->k_value[frame_index] = MAX(current_val, child_val + parent_padding);
            }
        }
    }

    // interpolate between frames
    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        for (int attr = 0; attr < NUM_ATTR; attr++) {
            int frame_start = geo_id * (page->max_keyframe * NUM_ATTR) + attr * page->max_keyframe;

            if (!graphics_keyframe_exists(&have_keyframe[frame_start], page->max_keyframe)) {
                continue;
            }

            graphics_keyframe_interpolate_frames(
                &page->k_value[frame_start], 
                &have_keyframe[frame_start], 
                page->max_keyframe
            );
        }
    }

    if (!LOG_KEYFRAMES) {
        return;
    }

    // print keyframes
    log_file(LogMessage, "Graphics", "Calculating Keyframes");

    for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
        log_file(LogMessage, "Graphics", "Geo ID %d: ", geo_id); 

        for (int attr = 0; attr < NUM_ATTR; attr++) {
            if (!page->attr_keyframe[geo_id * NUM_ATTR + attr]) {
                continue;
            }

            log_file(LogMessage, "Graphics", "\tAttr %s: ", ATTR[attr]);

            for (int frame_index = 0; frame_index <= page->max_keyframe; frame_index++) {
                int i = geo_id * (page->max_keyframe * NUM_ATTR) + attr * page->max_keyframe + frame_index;

                log_file(LogMessage, "Graphics", "\t\tFrame %d: %d", frame_index, page->k_value[i]); 
            }
        }
    }
}

int graphics_keyframe_interpolate_int(int v_start, int v_end, int index, int width) {
    if (width == 0) {
        log_file(LogWarn, "Graphics", "Interpolating over an interval of length 0");
        return v_start;
    }

    return (v_end - v_start) * index / width + v_start;
}

void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames) {
    int start = 0;
    int end;

    while (start < num_frames) { 
        if (!frames[start]) {
            start++;
            continue;
        }

        for (end = start + 1; end < num_frames; end++) {
            if (!frames[end]) {
                continue;
            }

            break;
        }

        if (!frames[end]) {
            break;
        }

        for (int i = start + 1; i < end; i++) {
            values[i] = graphics_keyframe_interpolate_int(values[start], values[end], i, end - start);
        }

        start = end;
    }
}

