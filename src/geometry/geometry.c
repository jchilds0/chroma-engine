/*
*
*/

#include "chroma-engine.h"
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

    } else if (strncmp(type, "annulus", 7) == 0) {

        return (IGeometry *) geometry_new_annulus();

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
            break;
        case CIRCLE:
            break;
        case ANNULUS:
            break;
        case TEXT:
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

GeometryAttr geometry_char_to_attr(char *attr) {
    GeometryAttr g_attr;

    if (strncmp(attr, "color", 5) == 0) {
        g_attr = GEO_COLOR;
    } else if (strncmp(attr, "pos_x", 5) == 0) {
        g_attr = GEO_POS_X;
    } else if (strncmp(attr, "pos_y", 5) == 0) {
        g_attr = GEO_POS_Y;
    } else if (strncmp(attr, "width", 5) == 0) {
        g_attr = GEO_WIDTH;
    } else if (strncmp(attr, "height", 6) == 0) {
        g_attr = GEO_HEIGHT;
    } else if (strncmp(attr, "center_x", 8) == 0) {
        g_attr = GEO_CENTER_X;
    } else if (strncmp(attr, "center_y", 8) == 0) {
        g_attr = GEO_CENTER_Y;
    } else if (strncmp(attr, "radius", 6) == 0) {
        g_attr = GEO_RADIUS;
    } else if (strncmp(attr, "inner_radius", 12) == 0) {
        g_attr = GEO_INNER_RADIUS;
    } else if (strncmp(attr, "outer_radius", 12) == 0) {
        g_attr = GEO_OUTER_RADIUS;
    } else if (strncmp(attr, "string", 4) == 0) {
        g_attr = GEO_TEXT;
    } else if (strncmp(attr, "scale", 5) == 0) {
        g_attr = GEO_SCALE;
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
            case ANNULUS:
                memcpy(value, "annulus", 7);
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

    switch (geo->geo_type) {
        case RECT:
            geometry_rectangle_get_attr((GeometryRect *)geo, g_attr, value);
            break;
        case CIRCLE:
            geometry_circle_get_attr((GeometryCircle *)geo, g_attr, value);
            break;
        case ANNULUS:
            geometry_annulus_get_attr((GeometryAnnulus *)geo, g_attr, value);
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

    switch (geo->geo_type) {
        case RECT:
            geometry_rectangle_set_attr((GeometryRect *)geo, g_attr, value);
            break;
        case CIRCLE:
            geometry_circle_set_attr((GeometryCircle *)geo, g_attr, value);
            break;
        case ANNULUS:
            geometry_annulus_set_attr((GeometryAnnulus *)geo, g_attr, value);
            break;
        case TEXT:
            geometry_text_set_attr((GeometryText *)geo, g_attr, value);
            break;
        default:
            log_file(LogWarn, "[Geometry]", "Unknown geo type %d", geo->geo_type);
    }
}

