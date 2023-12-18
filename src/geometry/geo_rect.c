/*
 *
 */

#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>


GeometryRect *geometry_new_rectangle(void) {
    GeometryRect *rect = NEW_STRUCT(GeometryRect);
    rect->geo_type = RECT;
    rect->pos_x = 0;
    rect->pos_y = 0;
    rect->width = 0;
    rect->height= 0;

    rect->color[0] = 0.0;
    rect->color[1] = 0.0;
    rect->color[2] = 0.0;
    rect->color[3] = 0.0;

    return rect;
}

void geometry_rectangle_get_attr(GeometryRect *rect, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    rect->color[0], 
                    rect->color[1], 
                    rect->color[2], 
                    rect->color[3]);
            break;
        case GEO_POS_X:
            sprintf(value, "%d", rect->pos_x);
            break;
        case GEO_POS_Y:
            sprintf(value, "%d", rect->pos_y);
            break;
        case GEO_WIDTH:
            sprintf(value, "%d", rect->width);
            break;
        case GEO_HEIGHT:
            sprintf(value, "%d", rect->height);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a rect attr (%d)", attr);
    }
}

void geometry_rectangle_set_attr(GeometryRect *rect, GeometryAttr attr, char *value) {
    int g_value = atoi(value);
    int r, g, b, a;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%d %d %d %d", &r, &g, &b, &a);
            rect->color[0] = r * 1.0 / 255;
            rect->color[1] = g * 1.0 / 255;
            rect->color[2] = b * 1.0 / 255;
            rect->color[3] = a * 1.0 / 255;
            break;
        case GEO_POS_X:
            rect->pos_x = g_value;
            break;
        case GEO_POS_Y:
            rect->pos_y = g_value;
            break;
        case GEO_WIDTH:
            rect->width = g_value;
            break;
        case GEO_HEIGHT:
            rect->height = g_value;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a rect attr (%d)", attr);
    }
}

