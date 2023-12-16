/*
 *
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "page.h"

Page *init_page(uint num_rect, uint num_text, uint num_circle, uint num_annulus) {
    Page *page = NEW_STRUCT(Page);
    page->num_rect = num_rect;
    page->rect = NEW_ARRAY(page->num_rect, ChromaRectangle);

    page->num_text = num_text;
    page->text = NEW_ARRAY(page->num_text, ChromaText);

    page->num_circle = num_circle;
    page->circle = NEW_ARRAY(page->num_circle, ChromaCircle);

    page->num_annulus = num_annulus;
    page->annulus = NEW_ARRAY(page->num_annulus, ChromaAnnulus);

    page->page_animate_on = animate_none;
    page->page_continue = animate_none;
    page->page_animate_off = animate_none;

    page_set_color(&page->mask.color[0], 0, 0, 0, 255);
    page->mask.pos_x = 0;
    page->mask.pos_y = 0;
    page->mask.width = 0;
    page->mask.height = 0;

    GLfloat id[] = GL_MATH_ID;

    for (int i = 0; i < num_text; i++) {
        page_set_color(&page->text[i].color[0], 255, 255, 255, 255);
        page->text[i].pos_x = -1;
        page->text[i].pos_y = -1;

        for (int j = 0; j < 16; j++) {
            page->text[i].transform[j] = id[j];
        }

        memset(&page->text[i].do_transform, 0, MAX_BUF_SIZE);
    }

    return page;
}

void free_page(Page *page) {
    free(page->rect);
    free(page->text);
    free(page);
}

void page_set_color(GLfloat *color, GLuint r, GLuint g, GLuint b, GLuint a) {
    color[0] = r * 1.0f / 255;
    color[1] = g * 1.0f / 255;
    color[2] = b * 1.0f / 255;
    color[3] = a * 1.0f / 255;
}

static int geo_num = -1;
static ChromaGraphics geo = -1;

void page_set_page_attrs(Page *page, char *attr, char *value) {
    if (page_update_geo(attr, value)) {
        return;
    }

    switch (geo) {
        case RECT:
            if (geo_num >= page->num_rect) {
                log_file(LogWarn, "Rect index %d out of range", geo_num);
                return;
            }

            page_rect_set_attr(&page->rect[geo_num], attr, value);
            break;
        case CIRCLE:
            if (geo_num >= page->num_circle) {
                log_file(LogWarn, "Circle index %d out of range", geo_num);
                return;
            }

            page_circle_set_attr(&page->circle[geo_num], attr, value);
            break;
        case ANNULUS:
            if (geo_num >= page->num_annulus) {
                log_file(LogWarn, "Annulus index %d out of range", geo_num);
                return;
            }

            page_annulus_set_attr(&page->annulus[geo_num], attr, value);
            break;
        case TEXT:
            if (geo_num >= page->num_text) {
                log_file(LogWarn, "Text index %d out of range", geo_num);
                return;
            }

            page_text_set_attr(&page->text[geo_num], attr, value);
            break;
        default:
            log_file(LogWarn, "Unknown shape type %d", geo);
    }
}

int page_update_geo(char *attr, char *value) {
    if (strncmp(attr, "rect", 4) == 0) {
        geo_num = atoi(value);
        geo = RECT;
        return 1;
    } else if (strncmp(attr, "text", 4) == 0) {
        geo_num = atoi(value);
        geo = TEXT;
        return 1;
    } else if (strncmp(attr, "circle", 4) == 0) {
        geo_num = atoi(value);
        geo = CIRCLE;
        return 1;
    } else if (strncmp(attr, "annulus", 4) == 0) {
        geo_num = atoi(value);
        geo = ANNULUS;
        return 1;
    } else if (geo_num == -1) {
        log_file(LogWarn, "Missing geo num");
        return 1;
    } 

    return 0;
}

