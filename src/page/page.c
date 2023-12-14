/*
 *
 */

#include "chroma-engine.h"
#include <string.h>
#include "page.h"

Page *init_page(int num_rect, int num_text) {
    Page *page = NEW_STRUCT(Page);
    page->num_rect = num_rect;
    page->num_text = num_text;
    page->rect = NEW_ARRAY(num_rect, Chroma_Rectangle);
    page->text = NEW_ARRAY(num_text, Chroma_Text);
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

void page_set_page_attrs(Page *page, char *attr, char *value) {
    static int geo_num = -1;
    static int typed = -1;

    if (strncmp(attr, "rect", 4) == 0) {
        sscanf(value, "%d", &geo_num);
        typed = 0;
        return;
    } else if (strncmp(attr, "text", 4) == 0) {
        sscanf(value, "%d", &geo_num);
        typed = 1;
        return;
    } else if (geo_num == -1) {
        log_file(LogWarn, "Missing geo num");
        return;
    } 

    switch (typed) {
        case 0:
            // rectangle
            if (geo_num >= page->num_rect) {
                log_file(LogWarn, "Rect index %d out of range", geo_num);
                return;
            }

            page_rect_set_attr(&page->rect[geo_num], attr, value);
            break;
        case 1:
            // text 
            if (geo_num >= page->num_text) {
                log_file(LogWarn, "Text index %d out of range", geo_num);
                return;
            }

            page_text_set_attr(&page->text[geo_num], attr, value);
            break;
        default:
            log_file(LogWarn, "Unknown shape type %d", typed);
    }
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


