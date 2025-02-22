/*
 * geo_image.c 
 *
 * Functions for the GeometryImage struct which
 * describes a rectangular image.
 */

#include "geometry_internal.h"
#include <stdio.h>

GeometryImage *geometry_new_image(Arena *a) {
    GeometryImage *image = ARENA_ALLOC(a, GeometryImage);
    image->geo.geo_type = IMAGE;
    geometry_clean_image(image);

    return image;
}

void geometry_clean_image(GeometryImage *image) {
    image->scale = 1.0;
    image->image_id = -1;
    image->w = 0;
    image->h = 0;
    image->data = NULL;
}

void geometry_image_get_attr(GeometryImage *image, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_SCALE:
            sprintf(value, "%f", image->scale);
            break;
        case GEO_WIDTH:
            sprintf(value, "%d", (int) (image->scale * image->w));
            break;
        case GEO_HEIGHT:
            sprintf(value, "%d", (int) (image->scale * image->h));
            break;
        case GEO_IMAGE_ID:
            sprintf(value, "%d", image->image_id);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an image attr: %s", geometry_attr_to_char(attr));
            break;
    }
}

void geometry_image_set_attr(GeometryImage *image, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_SCALE:
            image->scale = atof(value);
            break;
        case GEO_WIDTH:
            image->w = atoi(value);
            break;
        case GEO_HEIGHT:
            image->h = atoi(value);
            break;
        case GEO_IMAGE_ID:
            image->image_id = atoi(value);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an image attr: %s", geometry_attr_to_char(attr));
            break;
    }
}

