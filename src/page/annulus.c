/*
 * Annulus page attr
 */

#include "chroma-engine.h"
#include "page.h"

void page_annulus_set_attr(ChromaAnnulus *annulus, char *attr, char *value) {
    if (strcmp(attr, "center_x") == 0) {

        annulus->center_x = atoi(value);

    } else if (strcmp(attr, "center_y") == 0) {

        annulus->center_y = atoi(value);

    } else if (strcmp(attr, "inner_radius") == 0) {

        annulus->inner_radius = atoi(value);

    } else if (strcmp(attr, "outer_radius") == 0) {

        annulus->outer_radius = atoi(value);

    } else {
        
        log_file(LogWarn, "Unknown rect attr (%s)", attr);

    }
}
