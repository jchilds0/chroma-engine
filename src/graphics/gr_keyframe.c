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

void graphics_page_add_keyframe_value(IPage *page, int frame_num, 
    int frame_geo, char frame_attr[GEO_BUF_SIZE], int value) {
    if (page->num_keyframe == page->len_keyframe) {
        log_file(LogWarn, "Graphics", "Out of keyframe memory");
        return;
    }

    Keyframe *frame = &page->keyframe[page->num_keyframe];

    frame->frame_num = frame_num;
    frame->geo_id = frame_geo;
    memcpy(frame->attr, frame_attr, GEO_BUF_SIZE);
    frame->type = SET_VALUE;

    frame->value = value;
    page->num_keyframe++;
}

void graphics_page_add_keyframe_user(IPage *page, int frame_num, 
    int frame_geo, char frame_attr[GEO_BUF_SIZE]) {
    if (page->num_keyframe == page->len_keyframe) {
        log_file(LogWarn, "Graphics", "Out of keyframe memory");
        return;
    }

    Keyframe *frame = &page->keyframe[page->num_keyframe];

    frame->frame_num = frame_num;
    frame->geo_id = frame_geo;
    memcpy(frame->attr, frame_attr, GEO_BUF_SIZE);
    frame->type = USER_VALUE;

    page->num_keyframe++;
}

void graphics_page_add_keyframe_bind(IPage *page, int frame_num, int frame_geo, 
     char frame_attr[GEO_BUF_SIZE], int bind_frame, int bind_geo, char bind_attr[GEO_BUF_SIZE]) {
    if (page->num_keyframe == page->len_keyframe) {
        log_file(LogWarn, "Graphics", "Out of keyframe memory");
        return;
    }

    Keyframe *frame = &page->keyframe[page->num_keyframe];

    frame->frame_num = frame_num;
    frame->geo_id = frame_geo;
    memcpy(frame->attr, frame_attr, GEO_BUF_SIZE);
    frame->type = BIND_VALUE;

    frame->bind_frame_num = bind_frame;
    frame->bind_geo_id = bind_geo;
    memcpy(frame->bind_attr, bind_attr, GEO_BUF_SIZE);
    
    page->num_keyframe++;
}

void graphics_page_keyframe(IPage *page) {
    page->max_keyframe = 0;

    for (int i = 0; i < page->len_keyframe; i++) {
        page->max_keyframe = MAX(page->max_keyframe, page->keyframe[i].frame_num);
    }

    free(page->k_value);
    page->k_value = NEW_ARRAY(page->max_keyframe * page->len_geometry * NUM_ATTR, int);
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

        frame_index = frame->geo_id * (page->max_keyframe * NUM_ATTR) 
            + attr * page->max_keyframe + frame->frame_num;
        have_keyframe[frame_index] = 1;
        page->attr_keyframe[frame->geo_id * NUM_ATTR + attr] = 1;

        switch (frame->type) {
            case USER_VALUE:
                geo = page->geometry[frame->geo_id];
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
}

int graphics_keyframe_interpolate_int(int v_start, int v_end, int index, int width) {
    if (width == 0) {
        log_file(LogWarn, "Graphics", "Interpolating over an interval of length 0");
        return v_start;
    }

    return (v_end - v_start) * index / width;
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

        for (int i = start; i < end; i++) {
            values[i] = graphics_keyframe_interpolate_int(values[start], values[end], i, end - start);
        }

        start = end;
    }
}

