
#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>


GeometryAnnulus *geometry_new_annulus(void) {
    GeometryAnnulus *annulus = NEW_STRUCT(GeometryAnnulus);
    annulus->geo_type = ANNULUS;
    annulus->center_x = 0;
    annulus->center_y = 0;
    annulus->inner_radius = 0;
    annulus->outer_radius = 0;

    annulus->color[0] = 0.0;
    annulus->color[1] = 0.0;
    annulus->color[2] = 0.0;
    annulus->color[3] = 0.0;

    return annulus;
}

void geometry_free_annulus(GeometryAnnulus *annulus) {
    free(annulus);
}

void geometry_annulus_get_attr(GeometryAnnulus *annulus, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    annulus->color[0], 
                    annulus->color[1], 
                    annulus->color[2], 
                    annulus->color[3]);
            break;
        case GEO_CENTER_X:
            sprintf(value, "%d", annulus->center_x);
            break;
        case GEO_CENTER_Y:
            sprintf(value, "%d", annulus->center_y);
            break;
        case GEO_INNER_RADIUS:
            sprintf(value, "%d", annulus->inner_radius);
            break;
        case GEO_OUTER_RADIUS:
            sprintf(value, "%d", annulus->outer_radius);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an annulus attr (%d)", attr);
    }
}

void geometry_annulus_set_attr(GeometryAnnulus *annulus, GeometryAttr attr, char *value) {
    int g_value = atoi(value);
    int r, g, b, a;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%d %d %d %d", &r, &g, &b, &a);
            annulus->color[0] = r * 1.0 / 255;
            annulus->color[1] = g * 1.0 / 255;
            annulus->color[2] = b * 1.0 / 255;
            annulus->color[3] = a * 1.0 / 255;
            break;
        case GEO_CENTER_X:
            annulus->center_x = g_value;
            break;
        case GEO_CENTER_Y:
            annulus->center_y = g_value;
            break;
        case GEO_INNER_RADIUS:
            annulus->inner_radius = g_value;
            break;
        case GEO_OUTER_RADIUS:
            annulus->outer_radius = g_value;
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an annulus attr (%d)", attr);
    }

}

