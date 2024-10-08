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

#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void geometry_get_attr(IGeometry *geo, char *attr, char *value);
void geometry_set_attr(IGeometry *geo, char *attr, char *value);

GeometryType geometry_geo_type(char *name) {
    if (strcmp(name, "Rectangle") == 0) {
        return RECT;
    } else if (strcmp(name, "Text") == 0) {
        return TEXT;
    } else if (strcmp(name, "Circle") == 0) {
        return CIRCLE;
    } else if (strcmp(name, "Image") == 0) {
        return IMAGE;
    } else if (strcmp(name, "Polygon") == 0) {
        return POLYGON;
    } else if (strcmp(name, "Clock") == 0) {
        return TEXT;
    } else if (strcmp(name, "List") == 0) {
        return TEXT;
    }

    return -1;
}

IGeometry *geometry_create_geometry(Arena *a, GeometryType type) {
    IGeometry *geo;

    switch (type) {
        case RECT:
            geo = (IGeometry *) geometry_new_rectangle(a);
            break;

        case CIRCLE:
            geo = (IGeometry *) geometry_new_circle(a);
            break;

        case TEXT:
            geo = (IGeometry *) geometry_new_text(a);
            break;

        case GRAPH:
            geo = (IGeometry *) geometry_new_graph(a);
            break;

        case IMAGE:
            geo = (IGeometry *) geometry_new_image(a);
            break;

        case POLYGON:
            geo = (IGeometry *) geometry_new_polygon(a);
            break;

        default:
            log_file(LogWarn, "Geometry", "Unknown geometry type (%d)", type);
            return NULL;
    }

    geo->geo_id = 0;
    geo->parent_id = 0;
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

void geometry_clean_geo(IGeometry *geo) {
    geo->geo_id = 0;
    geo->parent_id = 0;
    geo->pos.x = 0;
    geo->pos.y = 0;
    geo->rel.x = 0;
    geo->rel.y = 0;

    geo->mask_geo = 0;

    geo->bound_lower.x = 0;
    geo->bound_lower.y = 0;
    geo->bound_upper.x = 0;
    geo->bound_upper.y = 0;

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

        case POLYGON:
            geometry_clean_polygon((GeometryPolygon *)geo);
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
    } else if (strncmp(attr, "point", 5) == 0) {
        g_attr = GEO_POINT;
    } else if (strncmp(attr, "num_points", 9) == 0) {
        g_attr = GEO_NUM_POINTS;
    } else if (strncmp(attr, "graph_type", 10) == 0) {
        g_attr = GEO_GRAPH_TYPE;
    } else if (strncmp(attr, "image_id", 8) == 0) {
        g_attr = GEO_IMAGE_ID;
    } else if (strncmp(attr, "x_lower", 7) == 0) {
        g_attr = GEO_X_LOWER;
    } else if (strncmp(attr, "x_upper", 7) == 0) {
        g_attr = GEO_X_UPPER;
    } else if (strncmp(attr, "y_lower", 7) == 0) {
        g_attr = GEO_Y_LOWER;
    } else if (strncmp(attr, "y_upper", 7) == 0) {
        g_attr = GEO_Y_UPPER;
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
            sprintf(value, "%d", geo->parent_id);
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

        case POLYGON:
            geometry_polygon_get_attr((GeometryPolygon *)geo, attr, value);
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

            case POLYGON:
                memcpy(value, "polygon", 7);
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
            sscanf(value, "%d", &geo->parent_id);
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
        
        case POLYGON:
            geometry_polygon_set_attr((GeometryPolygon *)geo, attr, value);
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

void geometry_set_color(IGeometry *geo, float color, int index) {
    switch (geo->geo_type) {
        case RECT:
            ((GeometryRect *)geo)->color[index] = color;
            break;

        case CIRCLE:
            ((GeometryCircle*)geo)->color[index] = color;
            break;

        case GRAPH:
            ((GeometryGraph*)geo)->color[index] = color;
            break;

        case TEXT:
            ((GeometryText*)geo)->color[index] = color;
            break;

        case POLYGON:
            ((GeometryPolygon*)geo)->color[index] = color;
            break;

        default:
            log_file(LogWarn, "Geometry", "Geo type %d does not have a color attribute", geo->geo_type);
    }

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

        case POLYGON:
            break;

        default:
            log_file(LogWarn, "Geometry", "Graph add values not implemented for %d", geo->geo_type);
    }
}
