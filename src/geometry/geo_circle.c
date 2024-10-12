/*
 * geo_circle.c 
 *
 * Functions for the GeometryCircle struct which 
 * describes an circle with a given inner radius,
 * outer radius, starting angle and end angle.
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "geometry_internal.h"
#include "log.h"
#include <math.h>

GeometryCircle *geometry_new_circle(Arena *a) {
    GeometryCircle *circle = ARENA_ALLOC(a, GeometryCircle);
    circle->geo.geo_type = CIRCLE;
    geometry_clean_circle(circle);

    return circle;
}

void geometry_clean_circle(GeometryCircle *circle) {
    circle->inner_radius = 0;
    circle->outer_radius = 0;
    circle->start_angle = 0;
    circle->end_angle = 2 * M_PI;

    circle->color = (vec4){0, 0, 0, 0};
}

void geometry_circle_get_attr(GeometryCircle *circle, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    circle->color.x, 
                    circle->color.y, 
                    circle->color.z, 
                    circle->color.w);
            break;
        case GEO_INNER_RADIUS:
            sprintf(value, "%d", circle->inner_radius);
            break;
        case GEO_OUTER_RADIUS:
            sprintf(value, "%d", circle->outer_radius);
            break;
        case GEO_START_ANGLE:
            sprintf(value, "%d", (int)(circle->start_angle * 180 / M_PI));
            break;
        case GEO_END_ANGLE:
            sprintf(value, "%d", (int)(circle->end_angle * 180 / M_PI));
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an circle attr: %s", geometry_attr_to_char(attr));
    }
}

void geometry_circle_set_attr(GeometryCircle *circle, GeometryAttr attr, char *value) {
    int g_value = atoi(value);

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%f %f %f %f", 
                   &circle->color.x,
                   &circle->color.y,
                   &circle->color.z,
                   &circle->color.w);
            break;
        case GEO_INNER_RADIUS:
            circle->inner_radius = g_value;
            break;
        case GEO_OUTER_RADIUS:
            circle->outer_radius = g_value;
            break;
        case GEO_START_ANGLE:
            circle->start_angle = g_value * M_PI / 180;
            break;
        case GEO_END_ANGLE:
            circle->end_angle = g_value * M_PI / 180;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an circle attr: %s", geometry_attr_to_char(attr));
    }

}

