/*
 * geo_rect.c 
 *
 * Functions for the GeometryRect struct which
 * describes a rectangle.
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "geometry_internal.h"
#include "log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

GeometryRect *geometry_new_rectangle(Arena *a) {
    GeometryRect *rect = ARENA_ALLOC(a, GeometryRect);
    rect->geo.geo_type = RECT;
    geometry_clean_rect(rect);

    return rect;
}

void geometry_clean_rect(GeometryRect *rect) {
    rect->width = 0;
    rect->height = 0;

    rect->color = (vec4){0, 0, 0, 0};
    rect->rounding = 0;
}

void geometry_rectangle_get_attr(GeometryRect *rect, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR_R:
            sprintf(value, "%f", rect->color.x);
            break;
        case GEO_COLOR_G:
            sprintf(value, "%f", rect->color.y);
            break;
        case GEO_COLOR_B:
            sprintf(value, "%f", rect->color.z);
            break;
        case GEO_COLOR_A:
            sprintf(value, "%f", rect->color.w);
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
            log_file(LogWarn, "Geometry", "Geo attr not a rect attr: %s", geometry_attr_to_char(attr));
    }
}

void geometry_rectangle_set_attr(GeometryRect *rect, GeometryAttr attr, char *value) {
    int g_value = atoi(value);

    switch (attr) {
        case GEO_COLOR_R:
            rect->color.x = atof(value);
            break;
        case GEO_COLOR_G:
            rect->color.y = atof(value);
            break;
        case GEO_COLOR_B:
            rect->color.z = atof(value);
            break;
        case GEO_COLOR_A:
            rect->color.w = atof(value);
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
            log_file(LogWarn, "Geometry", "Geo attr not a rect attr: %s", geometry_attr_to_char(attr));
    }
}

