/*
 * geo_image.c 
 *
 * Functions for the GeometryImage struct which
 * describes a rectangular image.
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "geometry/geometry_internal.h"
#include "log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


GeometryImage *geometry_new_image(void) {
    GeometryImage *image = NEW_STRUCT(GeometryImage);
    image->geo.geo_type = IMAGE;
    geometry_clean_image(image);

    return image;
}

void geometry_free_image(GeometryImage *image) {
    free(image);
}

void geometry_clean_image(GeometryImage *image) {
    image->scale = 1.0;
    image->data = NULL;
    image->image_id = 0;
    image->w = 0;
    image->h = 0;
    image->cur_image_id = -1;
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
            log_file(LogWarn, "Geometry", "Geo attr not an image attr (%d)", attr);
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
            log_file(LogWarn, "Geometry", "Geo attr not an image attr (%d)", attr);
    }
}

