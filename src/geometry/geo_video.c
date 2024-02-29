/*
 * geo_video.c 
 *
 * Functions for the GeometryVideo struct which
 * describes a rectangular video.
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


GeometryVideo *geometry_new_video(void) {
    GeometryVideo *video = NEW_STRUCT(GeometryVideo);
    video->geo.geo_type = VIDEO;
    video->geo.pos.x = 0;
    video->geo.pos.y = 0;
    video->geo.rel.x = 0;
    video->geo.rel.y = 0;

    video->scale = 1.0;
    memset(video->path, '\0', GEO_BUF_SIZE);

    return video;
}

void geometry_free_video(GeometryVideo *video) {
    free(video);
}

void geometry_video_get_attr(GeometryVideo *video, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_SCALE:
            sprintf(value, "%f", video->scale);
            break;
        case GEO_TEXT:
            memcpy(value, video->path, GEO_BUF_SIZE);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an video attr (%d)", attr);
    }
}

void geometry_video_set_attr(GeometryVideo *video, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_SCALE:
            video->scale = atof(value);
            break;
        case GEO_TEXT:
            memcpy(video->path, value, GEO_BUF_SIZE);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not an video attr (%d)", attr);
    }
}

