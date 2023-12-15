/*
 *
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "page.h"

Page *init_page(int num_rect, int num_text, int num_circle, int num_annulus) {
    Page *page = NEW_STRUCT(Page);
    page->num_rect = num_rect;
    page->rect = NEW_ARRAY(page->num_rect, ChromaRectangle);

    page->num_text = num_text;
    page->text = NEW_ARRAY(page->num_text, ChromaText);

    page->num_circle = num_circle;
    page->circle = NEW_ARRAY(page->num_circle, ChromaCircle);

    page->num_annulus = num_annulus;
    page->annulus = NEW_ARRAY(page->num_annulus, ChromaAnnulus);

    page->mask_time = 0.0f;
    page->clock_time = 0.0f;

    page->page_animate = animate_none;
    page->page_continue = animate_none;

    page_set_color(&page->mask.color[0], 0, 0, 0, 255);

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

void page_animate_on(int page_num) {
    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        log_file(LogWarn, "Page number %d out of range", page_num);
        return;
    }

    engine.hub->current_page = page_num;
    Page *page = engine.hub->pages[page_num];

    page->page_animate(page_num);
    for (int i = 0; i < page->num_rect; i++) {
        gl_rect_render(&page->rect[i]);
    }

    for (int i = 0; i < page->num_circle; i++) {
        gl_circle_render(&page->circle[i]);
    }

    for (int i = 0; i < page->num_annulus; i++) {
        gl_annulus_render(&page->annulus[i]);
    }

    for (int i = 0; i < page->num_text; i++) {
        gl_text_render(&page->text[i], 1.0);
    }

    gl_rect_render(&page->mask);
}

void page_continue(int page_num) {
    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        return;
    }

    Page *page = engine.hub->pages[page_num];

    page->page_continue(page_num);
    for (int i = 0; i < page->num_rect; i++) {
        gl_rect_render(&page->rect[i]);
    }

    for (int i = 0; i < page->num_circle; i++) {
        gl_circle_render(&page->circle[i]);
    }

    for (int i = 0; i < page->num_annulus; i++) {
        gl_annulus_render(&page->annulus[i]);
    }

    for (int i = 0; i < page->num_text; i++) {
        gl_text_render(&page->text[i], 1.0);
    }
}

void page_animate_off(int page_num) {
    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        return;
    }
    
    //Page *page = engine.hub->pages[page_num];

    // for (int i = 0; i < page->num_rect; i++) {
    //     rect = &page->rect[i];
    // }
    //
    // for (int i = 0; i < 2; i++) {
    //     text = &page->text[i];
    // }
}


