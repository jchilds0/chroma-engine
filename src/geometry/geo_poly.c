/*
 * geo_poly.c 
 *
 * Functions for the GeometryPolygon struct which
 * describes a polygon of n points.
 */

#include "geometry_internal.h"
#include <stdio.h>
#include <stdlib.h>

GeometryPolygon *geometry_new_polygon(Arena *a) {
    GeometryPolygon *poly = ARENA_ALLOC(a, GeometryPolygon);
    poly->geo.geo_type = POLYGON;
    geometry_clean_polygon(poly);

    return poly;
}

void geometry_clean_polygon(GeometryPolygon *poly) {
    poly->num_vertices = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        poly->vertex[i].x = 0;
        poly->vertex[i].y = 0;
    }

    poly->color = (vec4){0, 0, 0, 0};
}

void geometry_polygon_get_attr(GeometryPolygon *poly, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR_R:
            sprintf(value, "%f", poly->color.x);
            break;
        case GEO_COLOR_G:
            sprintf(value, "%f", poly->color.y);
            break;
        case GEO_COLOR_B:
            sprintf(value, "%f", poly->color.z);
            break;
        case GEO_COLOR_A:
            sprintf(value, "%f", poly->color.w);
            break;
        case GEO_NUM_POINTS:
            sprintf(value, "%d", poly->num_vertices);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a poly attr: %s", geometry_attr_to_char(attr));
    }
}

vec2 geometry_polygon_get_point(GeometryPolygon *poly, int index) {
    if (index < 0 || index >= poly->num_vertices) {
        log_file(LogWarn, "Geometry", "Polygon: Index %d out of range", index);
        return (vec2) {0, 0};
    }

    return poly->vertex[index];
}

void geometry_polygon_set_attr(GeometryPolygon *poly, GeometryAttr attr, char *value) {
    int i, x, y;

    switch (attr) {
        case GEO_COLOR_R:
            poly->color.x = atof(value);
            break;
        case GEO_COLOR_G:
            poly->color.y = atof(value);
            break;
        case GEO_COLOR_B:
            poly->color.z = atof(value);
            break;
        case GEO_COLOR_A:
            poly->color.w = atof(value);
            break;
        case GEO_NUM_POINTS:
            poly->num_vertices = atoi(value);
            break;
        case GEO_POINT:
            sscanf(value, "%d %d %d", &i, &x, &y);
            poly->vertex[i].x = x;
            poly->vertex[i].y = y;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr is not a poly attr: %s", geometry_attr_to_char(attr));
    }

    if (poly->num_vertices > MAX_NODES) {
        log_file(LogWarn, "Geometry", "Polygon: Tried to specify too many nodes %d, expected at most %d", poly->num_vertices, MAX_NODES);
        poly->num_vertices = MAX_NODES;
    }
}

void geometry_polygon_set_point(GeometryPolygon *poly, vec2 vec, int index) {
    if (index < 0 || index >= poly->num_vertices) {
        log_file(LogWarn, "Geometry", "Polygon: Index %d out of range to set (%d, %d)", index, vec.x, vec.y);
        return;
    }

    poly->vertex[index] = vec;
}
