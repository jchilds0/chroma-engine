/*
 * geometry.c
 *
 * Definitions for the geometry API functions
 *
 *    extern IGeometry *geometry_create_geometry(char *);
 *    extern void geometry_free_geometry(IGeometry *geo);
 *
 *    extern void geometry_set_attr(IGeometry *geo, char *attr, char *value);
 *    extern void geometry_set_int_attr(IGeometry *geo, char *attr, int value);
 *    extern void geometry_get_attr(IGeometry *geo, char *attr, char *value);
 *    extern int  geometry_get_int_attr(IGeometry *geo, char *attr);
 *
 */

#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void geometry_get_attr(IGeometry *geo, char *attr, char *value);
void geometry_set_attr(IGeometry *geo, char *attr, char *value);

IGeometry *geometry_create_geometry(char *type) {
    if (strncmp(type, "rect", 4) == 0) {

        return (IGeometry *) geometry_new_rectangle();

    } else if (strncmp(type, "circle", 6) == 0) {

        return (IGeometry *) geometry_new_circle();

    } else if (strncmp(type, "graph", 5) == 0) {

        return (IGeometry *) geometry_new_graph();

    } else if (strncmp(type, "text", 4) == 0) {

        return (IGeometry *) geometry_new_text();

    } else {
        log_file(LogWarn, "Geometry", "Unknown geometry type (%s)", type);
        return NULL;
    }

}

void geometry_free_geometry(IGeometry *geo) {
    switch (geo->geo_type) {
        case RECT:
            geometry_free_rectangle((GeometryRect *)geo);
            break;
        case CIRCLE:
            geometry_free_circle((GeometryCircle *)geo);
            break;
        case GRAPH:
            geometry_free_graph((GeometryGraph *)geo);
            break;
        case TEXT:
            geometry_free_text((GeometryText *)geo);
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

GeometryAttr geometry_char_to_attr(char *attr) {
    GeometryAttr g_attr;

    if (strncmp(attr, "pos_x", 5) == 0) {
        g_attr = GEO_POS_X;
    } else if (strncmp(attr, "pos_y", 5) == 0) {
        g_attr = GEO_POS_Y;
    } else if (strncmp(attr, "rel_x", 5) == 0) {
        g_attr = GEO_REL_X;
    } else if (strncmp(attr, "rel_y", 5) == 0) {
        g_attr = GEO_REL_Y;
    } else if (strncmp(attr, "color", 5) == 0) {
        g_attr = GEO_COLOR;
    } else if (strncmp(attr, "width", 5) == 0) {
        g_attr = GEO_WIDTH;
    } else if (strncmp(attr, "height", 6) == 0) {
        g_attr = GEO_HEIGHT;
    } else if (strncmp(attr, "rounding", 8) == 0) {
        g_attr = GEO_ROUNDING;
    } else if (strncmp(attr, "inner_radius", 12) == 0) {
        g_attr = GEO_INNER_RADIUS;
    } else if (strncmp(attr, "outer_radius", 12) == 0) {
        g_attr = GEO_OUTER_RADIUS;
    } else if (strncmp(attr, "start_angle", 11) == 0) {
        g_attr = GEO_START_ANGLE;
    } else if (strncmp(attr, "end_angle", 9) == 0) {
        g_attr = GEO_END_ANGLE;
    } else if (strncmp(attr, "string", 4) == 0) {
        g_attr = GEO_TEXT;
    } else if (strncmp(attr, "scale", 5) == 0) {
        g_attr = GEO_SCALE;
    } else if (strncmp(attr, "graph_node", 10) == 0) {
        g_attr = GEO_GRAPH_NODE;
    } else if (strncmp(attr, "num_node", 8) == 0) {
        g_attr = GEO_NUM_NODE;
    } else if (strncmp(attr, "graph_type", 10) == 0) {
        g_attr = GEO_GRAPH_TYPE;
    } else {
        log_file(LogWarn, "Geometry", "Unknown geometry attr (%s)", attr);
        return -1;
    }

    return g_attr;
}

int geometry_get_int_attr(IGeometry *geo, char *attr) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    geometry_get_attr(geo, attr, buf);

    return atoi(buf);
}

void geometry_get_attr(IGeometry *geo, char *attr, char *value) {
    if (strncmp(attr, "geo_type", 8) == 0) {
        switch (geo->geo_type) {
            case RECT:
                memcpy(value, "rect", 4);
                break;
            case CIRCLE:
                memcpy(value, "circle", 6);
                break;
            case GRAPH:
                memcpy(value, "graph", 5);
                break;
            case TEXT:
                memcpy(value, "text", 4);
                break;
            default:
                log_file(LogWarn, "[Geometry]", "Unknown geo type %d", geo->geo_type);
        }

        return;
    }

    GeometryAttr g_attr = geometry_char_to_attr(attr);

    switch (g_attr) {
        case GEO_POS_X:
            sprintf(value, "%d", geo->pos.x);
            return;
        case GEO_POS_Y:
            sprintf(value, "%d", geo->pos.y);
            return;
        case GEO_REL_X:
            sprintf(value, "%d", geo->rel.x);
            return;
        case GEO_REL_Y:
            sprintf(value, "%d", geo->rel.y);
            return;
        default:
            break;
    }

    switch (geo->geo_type) {
        case RECT:
            geometry_rectangle_get_attr((GeometryRect *)geo, g_attr, value);
            break;
        case CIRCLE:
            geometry_circle_get_attr((GeometryCircle *)geo, g_attr, value);
            break;
        case GRAPH:
            geometry_graph_get_attr((GeometryGraph *)geo, g_attr, value);
            break;
        case TEXT:
            geometry_text_get_attr((GeometryText *)geo, g_attr, value);
            break;
        default:
            log_file(LogWarn, "[Geometry]", "Unknown geo type %d", geo->geo_type);
    }
}

void geometry_set_int_attr(IGeometry *geo, char *attr, int value) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    sprintf(buf, "%d", value);
    geometry_set_attr(geo, attr, buf);
}

void geometry_set_attr(IGeometry *geo, char *attr, char *value) {
    GeometryAttr g_attr = geometry_char_to_attr(attr);

    switch (g_attr) {
        case GEO_POS_X:
            sscanf(value, "%d", &geo->pos.x);
            return;
        case GEO_POS_Y:
            sscanf(value, "%d", &geo->pos.y);
            return;
        case GEO_REL_X:
            sscanf(value, "%d", &geo->rel.x);
            return;
        case GEO_REL_Y:
            sscanf(value, "%d", &geo->rel.y);
            return;
        default:
            break;
    }

    switch (geo->geo_type) {
        case RECT:
            geometry_rectangle_set_attr((GeometryRect *)geo, g_attr, value);
            break;
        case CIRCLE:
            geometry_circle_set_attr((GeometryCircle *)geo, g_attr, value);
            break;
        case GRAPH:
            geometry_graph_set_attr((GeometryGraph *)geo, g_attr, value);
            break;
        case TEXT:
            geometry_text_set_attr((GeometryText *)geo, g_attr, value);
            break;
        default:
            log_file(LogWarn, "[Geometry]", "Unknown geo type %d", geo->geo_type);
    }
}

