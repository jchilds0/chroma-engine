/*
 * Render text using OpenGL
 */

#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void geometry_text_get_attr(GeometryText *text, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    text->color[0], 
                    text->color[1], 
                    text->color[2], 
                    text->color[3]);
            break;
        case GEO_POS_X:
            sprintf(value, "%d", text->pos_x);
            break;
        case GEO_POS_Y:
            sprintf(value, "%d", text->pos_y);
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
    int g_value = atoi(value);
    float scale;
    int r, g, b, a;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%d %d %d %d", &r, &g, &b, &a);
            text->color[0] = r * 1.0 / 255;
            text->color[1] = g * 1.0 / 255;
            text->color[2] = b * 1.0 / 255;
            text->color[3] = a * 1.0 / 255;
            break;
        case GEO_POS_X:
            text->pos_x = g_value;
            break;
        case GEO_POS_Y:
            text->pos_y = g_value;
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

