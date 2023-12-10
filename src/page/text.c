/*
 * Text page attr
 */

#include "chroma-engine.h"
#include "page.h"

void page_text_set_attr(Chroma_Text *text, char *attr, char *value) {
    if (strcmp(attr, "string") == 0) {

        memcpy(text->buf, value, MAX_BUF_SIZE);

    } else if (strcmp(attr, "pos_x") == 0) {

        sscanf(value, "%d", &text->pos_x);

    } else if (strcmp(attr, "pos_y") == 0) {

        sscanf(value, "%d", &text->pos_y);

    } else {
        
        log_file(LogWarn, "Unknown text attr (%s)", attr);

    }
}
