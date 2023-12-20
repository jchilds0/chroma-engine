/*
 * Render a circle using OpenGL
 */

#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

GeometryCircle *geometry_new_circle(void) {
    GeometryCircle *circle = NEW_STRUCT(GeometryCircle);
    circle->geo_type = CIRCLE;
    circle->center_x = 0;
    circle->center_y = 0;
    circle->radius   = 0;

    circle->color[0] = 0.0;
    circle->color[1] = 0.0;
    circle->color[2] = 0.0;
    circle->color[3] = 0.0;

    return circle;
}

void geometry_free_circle(GeometryCircle *circle) {
    free(circle);
}

void geometry_circle_get_attr(GeometryCircle *circle, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    circle->color[0], 
                    circle->color[1], 
                    circle->color[2], 
                    circle->color[3]);
            break;
        case GEO_CENTER_X:
            sprintf(value, "%d", circle->center_x);
            break;
        case GEO_CENTER_Y:
            sprintf(value, "%d", circle->center_y);
            break;
        case GEO_RADIUS:
            sprintf(value, "%d", circle->radius);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a circle attr (%d)", attr);
    }
}

void geometry_circle_set_attr(GeometryCircle *circle, GeometryAttr attr, char *value) {
    int g_value = atoi(value);
    int r, g, b, a;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%d %d %d %d", &r, &g, &b, &a);
            circle->color[0] = r * 1.0 / 255;
            circle->color[1] = g * 1.0 / 255;
            circle->color[2] = b * 1.0 / 255;
            circle->color[3] = a * 1.0 / 255;
            break;
        case GEO_CENTER_X:
            circle->center_x = g_value;
            break;
        case GEO_CENTER_Y:
            circle->center_y = g_value;
            break;
        case GEO_RADIUS:
            circle->radius = g_value;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a circle attr (%d)", attr);
    }
}

