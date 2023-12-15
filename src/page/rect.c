/*
 * Rectangle page attr
 */

#include "chroma-engine.h"
#include "page.h"

void page_rect_set_attr(ChromaRectangle *rect, char *attr, char *value) {
    if (strcmp(attr, "pos_x") == 0) {

        rect->pos_x = atoi(value);

    } else if (strcmp(attr, "pos_y") == 0) {

        rect->pos_y = atoi(value);

    } else if (strcmp(attr, "width") == 0) {

        rect->width = atoi(value);

    } else if (strcmp(attr, "height") == 0) {

        rect->height = atoi(value);

    } else {
        
        log_file(LogWarn, "Unknown rect attr (%s)", attr);

    }
}
