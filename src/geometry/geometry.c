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
    IGeometry *geo;
    if (strcmp(type, "rect") == 0) {

        geo = (IGeometry *) geometry_new_rectangle();

    } else if (strcmp(type, "circle") == 0) {

        geo = (IGeometry *) geometry_new_circle();

    } else if (strcmp(type, "graph") == 0) {

        geo = (IGeometry *) geometry_new_graph();

    } else if (strcmp(type, "text") == 0) {

        geo = (IGeometry *) geometry_new_text();
    
    } else if (strcmp(type, "image") == 0) {

        geo = (IGeometry *) geometry_new_image();

    } else {
        log_file(LogWarn, "Geometry", "Unknown geometry type (%s)", type);
        return NULL;
    }

    geo->parent = 0;
    geo->pos.x = 0;
    geo->pos.y = 0;
    geo->rel.x = 0;
    geo->rel.y = 0;

    return geo;
}

IGeometry *geometry_create_copy(IGeometry *geo) {
    IGeometry *new_geo;

    new_geo->geo_type = geo->geo_type;
    new_geo->pos.x = geo->pos.x;
    new_geo->pos.y = geo->pos.y;
    new_geo->rel.x = geo->rel.x;
    new_geo->rel.y = geo->rel.y;

    switch (geo->geo_type) {
    case RECT:
        new_geo = (IGeometry *) geometry_copy_rectangle((GeometryRect *) geo);
        break;
    case CIRCLE:
        new_geo = (IGeometry *) geometry_copy_circle((GeometryCircle *) geo);
    case GRAPH:
        new_geo = (IGeometry *) geometry_copy_graph((GeometryGraph *) geo);
    case TEXT:
        new_geo = (IGeometry *) geometry_copy_text((GeometryText *) geo);
    case IMAGE:
        new_geo = (IGeometry *) geometry_copy_image((GeometryImage*) geo);
    default:
        log_file(LogWarn, "Geometry", "Unknown geometry type (%s)", type);
        return NULL;
    }

    return new_geo;
}


void geometry_free_geometry(IGeometry *geo) {
    if (geo == NULL) {
        return;
    }

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
        case IMAGE:
            geometry_free_image((GeometryImage *)geo);
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

void geometry_clean_geo(IGeometry *geo) {
    switch (geo->geo_type) {
        case RECT:
            geometry_clean_rect((GeometryRect *)geo);
            break;
        case CIRCLE:
            geometry_clean_circle((GeometryCircle *)geo);
            break;
        case GRAPH:
            geometry_clean_graph((GeometryGraph *)geo);
            break;
        case TEXT:
            geometry_clean_text((GeometryText *)geo);
            break;
        case IMAGE:
            geometry_clean_image((GeometryImage *)geo);
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
    } else if (strncmp(attr, "parent", 6) == 0) {
        g_attr = GEO_PARENT;
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
    } else if (strncmp(attr, "image_id", 8) == 0) {
        g_attr = GEO_IMAGE_ID;
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

float geometry_get_float_attr(IGeometry *geo, char *attr) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    geometry_get_attr(geo, attr, buf);

    return atof(buf);
}

void geometry_get_attr(IGeometry *geo, char *attr, char *value) {
    if (geo == NULL) {
        log_file(LogError, "Geometry", "Geometry is NULL");
    }

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
            case IMAGE:
                memcpy(value, "image", 5);
                break;
            default:
                log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
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
        case GEO_PARENT:
            sprintf(value, "%d", geo->parent);
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
        case IMAGE:
            geometry_image_get_attr((GeometryImage *)geo, g_attr, value);
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

void geometry_set_int_attr(IGeometry *geo, char *attr, int value) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    sprintf(buf, "%d", value);
    geometry_set_attr(geo, attr, buf);
}

void geometry_set_float_attr(IGeometry *geo, char *attr, float value) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    sprintf(buf, "%f", value);
    geometry_set_attr(geo, attr, buf);
}

void geometry_set_attr(IGeometry *geo, char *attr, char *value) {
    GeometryAttr g_attr = geometry_char_to_attr(attr);

    if (geo == NULL) {
        log_file(LogError, "Geometry", "Geometry is NULL");
    }

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
        case GEO_PARENT:
            sscanf(value, "%d", &geo->parent);
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
        case IMAGE:
            geometry_image_set_attr((GeometryImage *)geo, g_attr, value);
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

