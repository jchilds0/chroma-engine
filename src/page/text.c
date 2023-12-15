/*
 * Text page attr
 */

#include "chroma-engine.h"
#include "page.h"

void page_text_set_attr(ChromaText *text, char *attr, char *value) {
    if (strcmp(attr, "string") == 0) {

        memcpy(text->buf, value, MAX_BUF_SIZE);

    } else if (strcmp(attr, "pos_x") == 0) {

        text->pos_x = atoi(value);

    } else if (strcmp(attr, "pos_y") == 0) {

        text->pos_y = atoi(value);

    } else {
        
        log_file(LogWarn, "Unknown text attr (%s)", attr);

    }
}
