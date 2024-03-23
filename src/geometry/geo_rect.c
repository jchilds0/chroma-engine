/*
 * geo_rect.c 
 *
 * Functions for the GeometryRect struct which
 * describes a rectangle.
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
    rect->geo.geo_type = RECT;
    rect->geo.parent = 0;
    rect->geo.pos.x = 0;
    rect->geo.pos.y = 0;
    rect->geo.rel.x = 0;
    rect->geo.rel.y = 0;

    rect->width = 0;
    rect->height= 0;
    rect->color[0] = 0.0;
    rect->color[1] = 0.0;
    rect->color[2] = 0.0;
    rect->color[3] = 0.0;
    rect->rounding = 0;

    return rect;
}

void geometry_free_rectangle(GeometryRect *rect) {
    free(rect);
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
        case GEO_WIDTH:
            sprintf(value, "%d", rect->width);
            break;
        case GEO_HEIGHT:
            sprintf(value, "%d", rect->height);
            break;
        case GEO_ROUNDING:
            sprintf(value, "%d", rect->rounding);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a rect attr (%d)", attr);
    }
}

void geometry_rectangle_set_attr(GeometryRect *rect, GeometryAttr attr, char *value) {
    int g_value = atoi(value);

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%f %f %f %f", 
                   &rect->color[0],
                   &rect->color[1],
                   &rect->color[2],
                   &rect->color[3]);
            break;
        case GEO_WIDTH:
            rect->width = g_value;
            break;
        case GEO_HEIGHT:
            rect->height = g_value;
            break;
        case GEO_ROUNDING:
            rect->rounding = g_value;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a rect attr (%d)", attr);
    }
}

