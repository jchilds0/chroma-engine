/*
 * Circle page attr
 */

#include "chroma-engine.h"
#include "page.h"

void page_circle_set_attr(ChromaCircle *circle, char *attr, char *value) {
    if (strcmp(attr, "center_x") == 0) {

        circle->center_x = atoi(value);

    } else if (strcmp(attr, "center_y") == 0) {

        circle->center_y = atoi(value);

    } else if (strcmp(attr, "radius") == 0) {

        circle->radius = atoi(value);

    } else {
        
        log_file(LogWarn, "Unknown rect attr (%s)", attr);
    }
}

