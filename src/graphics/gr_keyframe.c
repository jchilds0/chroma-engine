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

void graphics_keyframe_store_value(IPage *page, Keyframe *frame, unsigned char *have_keyframe, unsigned char *expand);
void graphics_keyframe_default_frames(IPage *page, int frame_start, unsigned char *have_keyframe);
void graphics_keyframe_fill_tails(int *values, unsigned char *frames, int num_frames);
void graphics_keyframe_interpolate_frames(int *values, unsigned char *frames, int num_frames);
void graphics_keyframe_expand_child(IPage *page, int parent, unsigned char *expand);
void graphics_keyframe_expand_geometry(IPage *page, int parent, int attr, int keyframe, int child);
int  graphics_keyframe_geo_attr(IPage *page, int geo_id, int attr, int frame_index);

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
    unsigned char *have_keyframe = NEW_ARRAY(n, unsigned char);
    unsigned char *expand = NEW_ARRAY(n, unsigned char);

    for (int i = 0; i < n; i++) {
        have_keyframe[i] = 0;
        expand[i] = 0;
    }

    // store keyframe value
    {

        start = clock();
        for (int i = 0; i < page->len_keyframe; i++) {
            Keyframe *frame = &page->keyframe[i];

            graphics_keyframe_store_value(page, frame, have_keyframe, expand);
        }

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(
                LogMessage, 
                "Graphics", 
                "Parsed Keyframes in %f ms", 
                ((double) (end - start) * 1000) / CLOCKS_PER_SEC
            );
        }

    }

    // add default keyframes 
    {

        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Adding default keyframes");
        }

        start = clock();

        for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
            if (LOG_KEYFRAMES) {
                log_file(LogMessage, "Graphics", "\tGeo ID %d: ", geo_id);
            }

            for (int attr = 0; attr < GEO_NUM; attr++) {
                int frame_start = INDEX(geo_id, attr, 0, GEO_NUM, page->max_keyframe);

                if (!page->attr_keyframe[geo_id * GEO_NUM + attr]) {
                    continue;
                }

                if (LOG_KEYFRAMES) {
                    log_file(LogMessage, "Graphics", "\t\tAttr %d", attr);
                }

                graphics_keyframe_default_frames(page, frame_start, have_keyframe);
            }
        }

        end = clock();

        if (LOG_KEYFRAMES) {
            log_file(
                LogMessage, 
                "Graphics", 
                "Added default frames in %f ms", 
                ((double) (end - start) * 1000) / CLOCKS_PER_SEC
            );
        }

    }

    // expand for child geometries 
    start = clock();

    graphics_keyframe_expand_child(page, 0, expand);

    end = clock();
    if (LOG_KEYFRAMES) {
        log_file(
            LogMessage, 
            "Graphics", 
            "Processed expand frames in %f ms", 
            ((double) (end - start) * 1000) / CLOCKS_PER_SEC
        );
    }

    // interpolate between frames
    {

        start = clock();
        for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
            for (int attr = 0; attr < GEO_NUM; attr++) {
                int frame_start = INDEX(geo_id, attr, 0, GEO_NUM, page->max_keyframe);

                if (!page->attr_keyframe[geo_id * GEO_NUM + attr]) {
                    continue;
                }

                graphics_keyframe_interpolate_frames(
                    &page->k_value[frame_start], 
                    &have_keyframe[frame_start], 
                    page->max_keyframe
                );
            }
        }

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(
                LogMessage, 
                "Graphics", 
                "Interpolated frames in %f ms", 
                ((double) (end - start) * 1000) / CLOCKS_PER_SEC
            );
        }

    }

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

    free(have_keyframe);
    free(expand);
}

/*
 * Store values specified by keyframes in the 
 * page and record which geo/attr/frame we have 
 * a keyframe for and which attributes have 
 * expand frames
 */
void graphics_keyframe_store_value(IPage *page, Keyframe *frame, unsigned char *have_keyframe, unsigned char *expand) {
    IGeometry *geo = page->geometry[frame->geo_id];
    int frame_index = INDEX(frame->geo_id, frame->attr, frame->frame_num, GEO_NUM, page->max_keyframe);

    have_keyframe[frame_index] = 1;
    page->attr_keyframe[frame->geo_id * GEO_NUM + frame->attr] = 1;

    switch (frame->type) {
        case USER_FRAME:
            page->k_value[frame_index] = geometry_get_int_attr(geo, frame->attr);
            break;
        case SET_FRAME:
            page->k_value[frame_index] = frame->value;
            break;
        case BIND_FRAME:
            log_file(LogError, "Graphics", "Bind value not implemented");
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

    if (frame->expand) {
        expand[frame_index] = 1;
    }

    if (frame->mask) {
        IGeometry *geo = page->geometry[frame->geo_id];

        switch (frame->attr) {
            case GEO_REL_X:
                geometry_set_int_attr(geo, GEO_MASK_X, 1);
                break;
            case GEO_REL_Y:
                geometry_set_int_attr(geo, GEO_MASK_Y, 1);
                break;
            default:
                log_file(LogWarn, "Graphics", "Mask %d not implemented", frame->attr);
        }
    }
}

/*
 * Set the 0th keyframe to the same value 
 * as the first keyframe present, similarly
 * for the last keyframe and final frame.
 */
void graphics_keyframe_default_frames(IPage *page, int frame_start, unsigned char *have_keyframe) {
    int frame_index;
    // first keyframe
    for (frame_index = 0; !have_keyframe[frame_start + frame_index]; frame_index++);

    have_keyframe[frame_start] = 1;
    page->k_value[frame_start] = page->k_value[frame_start + frame_index];

    // last keyframe
    for (int i = 0; i < page->max_keyframe; i++) {
        if (!have_keyframe[frame_start + i]) {
            continue;
        }

        frame_index = i;
    };

    have_keyframe[frame_start + page->max_keyframe - 1] = 1;
    page->k_value[frame_start + page->max_keyframe - 1] = page->k_value[frame_start + frame_index];
}

// Not needed?
void graphics_keyframe_fill_tails(int *values, unsigned char *frames, int num_frames) {
    int start = 0;

    // back propagate the initial keyframe
    while (!frames[start]) { 
        start++; 
    };

    for (int i = 0; i < start; values[i++] = values[start]);

    for (int i = 0; i < num_frames; i++) {
        if (!frames[i]) {
            continue;
        }

        start = i;
    }

    // forward propagate the final keyframe
    for (int i = start + 1; i < num_frames; values[i++] = values[start]);
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

/*
 * For keyframes which specify expand, resize to 
 * fit child geometries.
 */
void graphics_keyframe_expand_child(IPage *page, int parent, unsigned char *expand) {
    int frame_index;

    for (int i = 1; i < page->len_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue;
        }

        if (page->geometry[i]->parent != parent) {
            continue;
        }

        graphics_keyframe_expand_child(page, i, expand);
    }

    if (LOG_KEYFRAMES) {
        log_file(LogMessage, "Graphics", "Expand geometry %d", parent);
    }

    for (int key_idx = 0; key_idx < page->max_keyframe; key_idx++) {
        for (int attr = 0; attr < GEO_NUM; attr++) {
            frame_index = INDEX(parent, attr, key_idx, GEO_NUM, page->max_keyframe);

            if (!expand[frame_index]) {
                continue; 
            }

            for (int child_id = 0; child_id < page->len_geometry; child_id++) {
                if (page->geometry[child_id] == NULL) {
                    continue;
                }

                if (page->geometry[child_id]->parent != parent) {
                    continue;
                }

                graphics_keyframe_expand_geometry(page, parent, attr, key_idx, child_id);
            }
        }
    }
}

/*
 * Calculate the expanded size of the geometry with 
 * index 'parent', for geometry attribute 'attr' at 
 * keyframe 'keyframe' to fit child geometry 'child'
 *
 * Assumes 'child' expansion has been calculated
 */
void graphics_keyframe_expand_geometry(IPage *page, int parent, int attr, int keyframe, int child) {
    IGeometry *geo = page->geometry[parent];
    int frame_index = INDEX(parent, attr, keyframe, GEO_NUM, page->max_keyframe);

    int current_val = page->k_value[frame_index];
    int parent_padding = geometry_get_int_attr(geo, attr);
    int child_val = graphics_keyframe_geo_attr(page, child, attr, keyframe);

    switch (attr) {
        case GEO_WIDTH:
            child_val += graphics_keyframe_geo_attr(page, child, GEO_REL_X, keyframe);
            break;

        case GEO_HEIGHT:
            child_val += graphics_keyframe_geo_attr(page, child, GEO_REL_Y, keyframe);
            break;

        default:
            log_file(LogWarn, "Graphics", "Expand %s not implemented", attr);
    }

    if (child_val + parent_padding > current_val && LOG_KEYFRAMES) {
        log_file(
            LogMessage, "Graphics", 
            "\tUpdated to expand for child %d to %d", 
            child, child_val + parent_padding);
    }

    page->k_value[frame_index] = MAX(current_val, child_val + parent_padding);
}

/*
 * Get a geometry attribute value. 
 *
 * If an attribute has a keyframe, return the 
 * keyframe value, otherwise return the geometry 
 * attribute value.
 */
int graphics_keyframe_geo_attr(IPage *page, int geo_id, int attr, int key_idx) {
    IGeometry *geo = page->geometry[geo_id];
    int geo_value = 0;
    int frame_index;

    if (page->attr_keyframe[geo_id * GEO_NUM + attr]) {
        frame_index = INDEX(geo_id, attr, key_idx, GEO_NUM, page->max_keyframe);
        geo_value = page->k_value[frame_index];
    } else {
        geo_value = geometry_get_int_attr(geo, attr);
    }

    return geo_value;
}
