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
#include <time.h>

void graphics_keyframe_expand_child(IPage *page, int parent, unsigned char *expand);
void graphics_keyframe_expand_geometry(IPage *page, int geo_id, unsigned char *expand);
int  graphics_keyframe_geo_attr(IPage *page, int geo_id, int attr, int frame_index);
void graphics_keyframe_fill_tails(int *values, unsigned char *frames, int num_frames);
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

    page->max_keyframe++;

    free(page->k_value);
    page->k_value = NEW_ARRAY(page->max_keyframe * page->len_geometry * GEO_NUM, int);
    for (int i = 0; i < page->max_keyframe * page->len_geometry * GEO_NUM; i++) {
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
    } else if (strcmp(name, "frame_attr") == 0) {
        frame->attr = value;
    } else if (strcmp(name, "value") == 0) {
        frame->type = SET_FRAME;
        frame->value = value;
    } else if (strcmp(name, "bind_frame") == 0) {
        frame->type = BIND_FRAME;
        frame->bind_frame_num = value;
    } else if (strcmp(name, "bind_geo") == 0) {
        frame->bind_geo_id = value;
    } else if (strcmp(name, "bind_attr") == 0) {
        frame->bind_attr = value;
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

    if (strcmp(name, "mask") == 0) {
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
        frame->type = USER_FRAME;
    } else {
        log_file(LogWarn, "Graphics", "Keyframe attr %s is not a string attr", name);
    }

}

void graphics_page_calculate_keyframes(IPage *page) {
    IGeometry *geo;
    int frame_index;
    int start, end;

    unsigned char *have_keyframe = NEW_ARRAY(page->max_keyframe * page->len_geometry * GEO_NUM, unsigned char);
    unsigned char *expand = NEW_ARRAY(page->max_keyframe * page->len_geometry * GEO_NUM, unsigned char);

    for (int i = 0; i < GEO_NUM * page->len_geometry * page->max_keyframe; i++) {
        have_keyframe[i] = 0;
        expand[i] = 0;
    }

    // store keyframe value
    {

        start = clock();
        for (int i = 0; i < page->len_keyframe; i++) {
            Keyframe *frame = &page->keyframe[i];
            geo = page->geometry[frame->geo_id];
            frame_index = frame->geo_id * (page->max_keyframe * GEO_NUM) 
                + frame->attr * page->max_keyframe + frame->frame_num;
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
                log_file(LogMessage, "Graphics", "\tInitially set geo %d, attr %s to %d", frame->geo_id, frame->attr, page->k_value[frame_index]);
            }

            if (frame->expand) {
                expand[frame_index] = 1;
            }
        }

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Parsed Keyframes in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
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
                int frame_start = geo_id * (page->max_keyframe * GEO_NUM) + attr * page->max_keyframe;

                if (!page->attr_keyframe[geo_id * GEO_NUM + attr]) {
                    continue;
                }

                if (LOG_KEYFRAMES) {
                    log_file(LogMessage, "Graphics", "\t\tAttr %d", attr);
                }

                // first keyframe
                for (frame_index = 0; !have_keyframe[frame_start + frame_index]; frame_index++);

                have_keyframe[frame_start] = 1;
                //expand[frame_start] = expand[frame_start + frame_index];
                page->k_value[frame_start] = page->k_value[frame_start + frame_index];

                // last keyframe

                for (int i = 0; i < page->max_keyframe; i++) {
                    if (!have_keyframe[frame_start + i]) {
                        continue;
                    }

                    frame_index = i;
                };

                have_keyframe[frame_start + page->max_keyframe] = 1;
                //expand[frame_start + page->max_keyframe] = expand[frame_start + frame_index];
                page->k_value[frame_start + page->max_keyframe] = page->k_value[frame_start + frame_index];
            }
        }

        end = clock();

        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Added default frames in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
        }

    }

    // expand for child geometries 
    start = clock();

    graphics_keyframe_expand_child(page, 0, expand);

    end = clock();
    if (LOG_KEYFRAMES) {
        log_file(LogMessage, "Graphics", "Processed expand frames in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
    }

    // interpolate between frames
    {

        start = clock();
        for (int geo_id = 0; geo_id < page->len_geometry; geo_id++) {
            for (int attr = 0; attr < GEO_NUM; attr++) {
                int frame_start = geo_id * (page->max_keyframe * GEO_NUM) + attr * page->max_keyframe;

                if (!page->attr_keyframe[geo_id * GEO_NUM + attr]) {
                    continue;
                }

                graphics_keyframe_interpolate_frames(
                    &page->k_value[frame_start], 
                    &have_keyframe[frame_start], 
                    page->max_keyframe
                );

                graphics_keyframe_fill_tails(
                    &page->k_value[frame_start], 
                    &have_keyframe[frame_start], 
                    page->max_keyframe
                );
            }
        }

        end = clock();
        if (LOG_KEYFRAMES) {
            log_file(LogMessage, "Graphics", "Interpolated frames in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
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
                int i = geo_id * (page->max_keyframe * GEO_NUM) + attr * page->max_keyframe + frame_index;

                log_file(LogMessage, "Graphics", "\t\t\tFrame %d: %d", frame_index, page->k_value[i]); 
            }
        }
    }

    free(have_keyframe);
    free(expand);
}

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

void graphics_keyframe_expand_child(IPage *page, int parent, unsigned char *expand) {
    for (int i = 1; i < page->len_geometry; i++) {
        if (page->geometry[i] == NULL) {
            continue;
        }

        if (page->geometry[i]->parent != parent) {
            continue;
        }

        graphics_keyframe_expand_child(page, i, expand);
    }

    graphics_keyframe_expand_geometry(page, parent, expand);
}

void graphics_keyframe_expand_geometry(IPage *page, int parent_id, unsigned char *expand_frames) {
    int frame_index;
    IGeometry *geo = page->geometry[parent_id];

    if (LOG_KEYFRAMES) {
        log_file(LogMessage, "Graphics", "Expand geometry %d", parent_id);
    }

    for (int key_idx = 0; key_idx < page->max_keyframe; key_idx++) {
        for (int attr = 0; attr < GEO_NUM; attr++) {
            frame_index = parent_id * (page->max_keyframe * GEO_NUM) 
                + attr * page->max_keyframe + key_idx;

            if (!expand_frames[frame_index]) {
                continue; 
            }

            for (int child_id = 0; child_id < page->num_geometry; child_id++) {
                if (page->geometry[child_id] == NULL) {
                    continue;
                }

                if (page->geometry[child_id]->parent != parent_id) {
                    continue;
                }

                int current_val = page->k_value[frame_index];
                int parent_padding = geometry_get_int_attr(geo, attr);
                int child_val = graphics_keyframe_geo_attr(page, child_id, attr, key_idx);

                switch (attr) {
                    case GEO_WIDTH:
                        child_val += graphics_keyframe_geo_attr(page, child_id, GEO_REL_X, key_idx);
                        break;

                    case GEO_HEIGHT:
                        child_val += graphics_keyframe_geo_attr(page, child_id, GEO_REL_Y, key_idx);
                        break;

                    default:
                        log_file(LogWarn, "Graphics", "Expand %s not implemented", attr);
                }

                if (child_val + parent_padding > current_val && LOG_KEYFRAMES) {
                    log_file(LogMessage, "Graphics", "\tUpdated to expand for child %d to %d", child_id, child_val + parent_padding);
                }

                page->k_value[frame_index] = MAX(current_val, child_val + parent_padding);
            }
        }
    }
    
}

int graphics_keyframe_geo_attr(IPage *page, int geo_id, int attr, int key_idx) {
    IGeometry *geo = page->geometry[geo_id];
    int geo_value = 0;
    int frame_index;

    if (page->attr_keyframe[geo_id * GEO_NUM + attr]) {
        frame_index = geo_id * (GEO_NUM * page->max_keyframe) 
            + attr * page->max_keyframe + key_idx; 

        geo_value = page->k_value[frame_index];
    } else {
        geo_value = geometry_get_int_attr(geo, attr);

    }

    return geo_value;
}
