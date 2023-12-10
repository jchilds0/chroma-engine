/*
 * Rectangle page attr
 */

#include "chroma-engine.h"
#include "page.h"

void page_rect_set_attr(Chroma_Rectangle *rect, char *attr, char *value) {
    if (strcmp(attr, "pos_x") == 0) {

        sscanf(value, "%d", &rect->pos_x);

    } else if (strcmp(attr, "pos_y") == 0) {

        sscanf(value, "%d", &rect->pos_y);

    } else if (strcmp(attr, "width") == 0) {

        sscanf(value, "%d", &rect->width);

    } else if (strcmp(attr, "height") == 0) {

        sscanf(value, "%d", &rect->height);

    } else {
        
        log_file(LogWarn, "Unknown rect attr (%s)", attr);

    }
}
