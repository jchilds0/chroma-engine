/*
 * geo_text.c 
 *
 * Functions for the GeometryText struct which 
 * describes text, namely the characters, scale
 * and color.
 */

#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


GeometryText *geometry_new_text(void) {
    GeometryText *text = NEW_STRUCT(GeometryText);
    text->geo.geo_type = TEXT;
    text->geo.pos.x = 0;
    text->geo.pos.y = 0;
    text->geo.rel.x = 0;
    text->geo.rel.y = 0;

    text->scale = 1.0;
    memset(text->buf, '\0', GEO_BUF_SIZE);

    text->color[0] = 0.0;
    text->color[1] = 0.0;
    text->color[2] = 0.0;
    text->color[3] = 0.0;

    return text;
}

void geometry_free_text(GeometryText *text) {
    free(text);
}

void geometry_text_get_attr(GeometryText *text, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    text->color[0], 
                    text->color[1], 
                    text->color[2], 
                    text->color[3]);
            break;
        case GEO_TEXT:
            memcpy(value, text->buf, GEO_BUF_SIZE);
            break;
        case GEO_SCALE:
            sprintf(value, "%f", text->scale);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a text attr (%d)", attr);
    }
}

void geometry_text_set_attr(GeometryText *text, GeometryAttr attr, char *value) {
    float scale;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%f %f %f %f", 
                   &text->color[0],
                   &text->color[1],
                   &text->color[2],
                   &text->color[3]);
            break;
        case GEO_TEXT:
            memmove(text->buf, value, GEO_BUF_SIZE);
            break;
        case GEO_SCALE:
            sscanf(value, "%f", &scale);
            text->scale = scale;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a text attr (%d)", attr);
    }
}

