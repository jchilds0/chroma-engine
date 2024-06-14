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

#include "geometry.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void geometry_get_attr(IGeometry *geo, char *attr, char *value);
void geometry_set_attr(IGeometry *geo, char *attr, char *value);

GeometryType geometry_char_to_type(char *name) {
    if (strncmp(name, "rect", MAX_NAME_LEN) == 0) {
        return RECT;
    } else if (strncmp(name, "text", MAX_NAME_LEN) == 0) {
        return TEXT;
    } else if (strncmp(name, "circle", MAX_NAME_LEN) == 0) {
        return CIRCLE;
    } else if (strncmp(name, "image", MAX_NAME_LEN) == 0) {
        return IMAGE;
    } 

    log_file(LogWarn, "Geometry", "Unknown geometry type (%d)", name);
    return -1;
}

IGeometry *geometry_create_geometry(GeometryType type) {
    IGeometry *geo;

    switch (type) {
        case RECT:
            geo = (IGeometry *) geometry_new_rectangle();
            break;

        case CIRCLE:
            geo = (IGeometry *) geometry_new_circle();
            break;

        case TEXT:
            geo = (IGeometry *) geometry_new_text();
            break;

        case GRAPH:
            geo = (IGeometry *) geometry_new_graph();
            break;

        case IMAGE:
            geo = (IGeometry *) geometry_new_image();
            break;

        default:
            log_file(LogWarn, "Geometry", "Unknown geometry type (%d)", type);
            return NULL;
    }

    geo->parent = 0;
    geo->pos.x = 0;
    geo->pos.y = 0;
    geo->rel.x = 0;
    geo->rel.y = 0;

    geo->mask_geo = 0;

    geo->bound_lower.x = 0;
    geo->bound_lower.y = 0;
    geo->bound_upper.x = 0;
    geo->bound_upper.y = 0;

    return geo;
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

unsigned char geometry_is_int_attr(GeometryAttr attr) {
    switch (attr) {
        case GEO_COLOR:
        case GEO_TEXT:
        case GEO_SCALE:
        case GEO_GRAPH_NODE:
        case GEO_NUM_NODE:
        case GEO_GRAPH_TYPE:
        case GEO_IMAGE_ID:
        case GEO_NUM:
            return 0;
        default:
            return 1;
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
    } else if (strncmp(attr, "mask", 4) == 0) {
        g_attr = GEO_MASK;
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

static void geometry_get_attribute(IGeometry *geo, GeometryAttr attr, char *value) {
    switch (attr) {
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
        case GEO_MASK:
            sprintf(value, "%d", geo->mask_geo);
            return;
        case GEO_X_LOWER:
            sprintf(value, "%d", geo->bound_lower.x);
            return;
        case GEO_X_UPPER:
            sprintf(value, "%d", geo->bound_upper.x);
            return;
        case GEO_Y_LOWER:
            sprintf(value, "%d", geo->bound_lower.y);
            return;
        case GEO_Y_UPPER:
            sprintf(value, "%d", geo->bound_upper.y);
            return;
        default:
            break;
    }

    switch (geo->geo_type) {
        case RECT:
            geometry_rectangle_get_attr((GeometryRect *)geo, attr, value);
            break;
        case CIRCLE:
            geometry_circle_get_attr((GeometryCircle *)geo, attr, value);
            break;
        case GRAPH:
            geometry_graph_get_attr((GeometryGraph *)geo, attr, value);
            break;
        case TEXT:
            geometry_text_get_attr((GeometryText *)geo, attr, value);
            break;
        case IMAGE:
            geometry_image_get_attr((GeometryImage *)geo, attr, value);
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

int geometry_get_int_attr(IGeometry *geo, GeometryAttr attr) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    geometry_get_attribute(geo, attr, buf);

    return atoi(buf);
}

float geometry_get_float_attr(IGeometry *geo, GeometryAttr attr) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    geometry_get_attribute(geo, attr, buf);

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
    geometry_get_attribute(geo, g_attr, value);
}

static void geometry_set_attribute(IGeometry *geo, GeometryAttr attr, char *value) {
    switch (attr) {
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
        case GEO_MASK:
            sscanf(value, "%d", &geo->mask_geo);
            return;
        case GEO_X_LOWER:
            sscanf(value, "%d", &geo->bound_lower.x);
            return;
        case GEO_X_UPPER:
            sscanf(value, "%d", &geo->bound_upper.x);
            return;
        case GEO_Y_LOWER:
            sscanf(value, "%d", &geo->bound_lower.y);
            return;
        case GEO_Y_UPPER:
            sscanf(value, "%d", &geo->bound_upper.y);
            return;
        default:
            break;
    }

    switch (geo->geo_type) {
        case RECT:
            geometry_rectangle_set_attr((GeometryRect *)geo, attr, value);
            break;
        case CIRCLE:
            geometry_circle_set_attr((GeometryCircle *)geo, attr, value);
            break;
        case GRAPH:
            geometry_graph_set_attr((GeometryGraph *)geo, attr, value);
            break;
        case TEXT:
            geometry_text_set_attr((GeometryText *)geo, attr, value);
            break;
        case IMAGE:
            geometry_image_set_attr((GeometryImage *)geo, attr, value);
            break;
        default:
            log_file(LogWarn, "Geometry", "Unknown geo type %d", geo->geo_type);
    }
}

void geometry_set_attr(IGeometry *geo, char *attr, char *value) {
    GeometryAttr g_attr = geometry_char_to_attr(attr);

    if (geo == NULL) {
        log_file(LogError, "Geometry", "Geometry is NULL");
    }

    geometry_set_attribute(geo, g_attr, value);
}

void geometry_set_int_attr(IGeometry *geo, GeometryAttr attr, int value) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    sprintf(buf, "%d", value);
    geometry_set_attribute(geo, attr, buf);
}

void geometry_set_float_attr(IGeometry *geo, GeometryAttr attr, float value) {
    char buf[GEO_BUF_SIZE];
    memset(buf, '\0', GEO_BUF_SIZE);
    sprintf(buf, "%f", value);
    geometry_set_attribute(geo, attr, buf);
}

void geometry_graph_add_values(IGeometry *geo, void (*add_value)(int)) {
    add_value(GEO_REL_X);
    add_value(GEO_REL_Y);

    switch (geo->geo_type) {
        case RECT:
            add_value(GEO_WIDTH);
            add_value(GEO_HEIGHT);
            break;

        case CIRCLE:
            add_value(GEO_START_ANGLE);
            add_value(GEO_END_ANGLE);
            add_value(GEO_INNER_RADIUS);
            add_value(GEO_OUTER_RADIUS);
            break;

        case TEXT:
            add_value(GEO_WIDTH);
            add_value(GEO_HEIGHT);
            break;

        case IMAGE:
            break;

        default:
            log_file(LogWarn, "Geometry", "Graph add values not implemented for %d", geo->geo_type);
    }
}
