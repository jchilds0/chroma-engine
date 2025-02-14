/*
 * geo_text.c 
 *
 * Functions for the GeometryText struct which 
 * describes text, namely the characters, scale
 * and color.
 */

#include "geometry_internal.h"
#include "gl_render.h"
#include <stdio.h>

GeometryText *geometry_new_text(Arena *a) {
    GeometryText *text = ARENA_ALLOC(a, GeometryText);
    text->geo.geo_type = TEXT;
    geometry_clean_text(text);

    return text;
}

void geometry_clean_text(GeometryText *text) {
    text->scale = 1.0;
    memset(text->buf, '\0', GEO_BUF_SIZE);

    text->color = (vec4){0, 0, 0, 0};
}

void geometry_text_get_attr(GeometryText *text, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR_R:
            sprintf(value, "%f", text->color.x);
            break;
        case GEO_COLOR_G:
            sprintf(value, "%f", text->color.y);
            break;
        case GEO_COLOR_B:
            sprintf(value, "%f", text->color.z);
            break;
        case GEO_COLOR_A:
            sprintf(value, "%f", text->color.w);
            break;
        case GEO_TEXT:
            memcpy(value, text->buf, GEO_BUF_SIZE);
            break;
        case GEO_SCALE:
            sprintf(value, "%f", text->scale);
            break;
        case GEO_WIDTH:
            sprintf(value, "%d", gl_text_text_width(text->buf, text->scale));
            break;
        case GEO_HEIGHT:
            sprintf(value, "%d", gl_text_text_height(text->buf, text->scale));
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a text attr: %s", geometry_attr_to_char(attr));
    }
}

void geometry_text_set_attr(GeometryText *text, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR_R:
            text->color.x = atof(value);
            break;
        case GEO_COLOR_G:
            text->color.y = atof(value);
            break;
        case GEO_COLOR_B:
            text->color.z = atof(value);
            break;
        case GEO_COLOR_A:
            text->color.w = atof(value);
            break;
        case GEO_TEXT:
            memmove(text->buf, value, GEO_BUF_SIZE);
            break;
        case GEO_SCALE:
            text->scale = atof(value);
            break;
        case GEO_WIDTH:
        case GEO_HEIGHT:
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a text attr: %s", geometry_attr_to_char(attr));
    }
}

