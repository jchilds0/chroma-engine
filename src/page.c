/*
 *
 */

#include "chroma-engine.h"
#include "chroma-prototypes.h"

Page *init_page(int num_rect) {
    Page *page = NEW_STRUCT(Page);
    page->num_rect = num_rect;
    page->rect = NEW_ARRAY(num_rect, Chroma_Rectangle);
    set_color(&page->text[0].color[0], 255, 255, 255, 255);
    set_color(&page->text[1].color[0], 255, 255, 255, 255);

    return page;
}

void free_page(Page *page) {
    free(page->rect);
    free(page);
}

void set_rect(Page *page, int rect_num, int pos_x, int pos_y, int width, int height) {
    if (!WITHIN(rect_num, 0, page->num_rect)) {
        return;
    }

    page->rect[rect_num] = (Chroma_Rectangle){pos_x, pos_y, width, height};
}

void set_color(GLfloat *color, GLuint r, GLuint g, GLuint b, GLuint a) {
    color[0] = r * 1.0f / 255;
    color[1] = g * 1.0f / 255;
    color[2] = b * 1.0f / 255;
    color[3] = a * 1.0f / 255;
}

void set_page_attr(Page *page, char *attr, char *value) {
    if (strcmp(attr, "title") == 0) {
        memcpy(page->text[0].buf, value, MAX_BUF_SIZE);
    } else if (strcmp(attr, "subtitle") == 0) {
        memcpy(page->text[1].buf, value, MAX_BUF_SIZE);
    } else {
        int value_int;
        sscanf(value, "%d#", &value_int);
        set_page_attr_int(page, attr, value_int);
    }
}

void set_page_attr_int(Page *page, char *attr, int value) {
    if (strcmp(attr, "pos_x") == 0) {
        page->rect[0].pos_x = value;
    } else if (strcmp(attr, "pos_y") == 0) {
        page->rect[0].pos_y = value;
    } else if (strcmp(attr, "width") == 0) {
        page->rect[0].width = value;
    } else if (strcmp(attr, "height") == 0) {
        page->rect[0].height = value;
    }
}

void animate_on_page(int page_num) {
    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        log_to_file(LogWarn, "Page number %d out of range", page_num);
        return;
    }

    engine.hub->current_page = page_num;
    Page *page = engine.hub->pages[page_num];
    Chroma_Text *text;

    for (int i = 0; i < page->num_rect; i++) {
        //DrawRectangle(rect->pos_x, rect->pos_y, rect->width, rect->height, rect->color);
        gl_rect_render(&page->rect[i]);
    }

    for (int i = 0; i < 2; i++) {
        text = &page->text[i];
        text->pos_x = page->rect[0].pos_x + page->rect[0].height / 3;
        text->pos_y = page->rect[0].pos_y + (2 - i) * page->rect[0].height / 3;
        
        //DrawText(text->buf, text->pos_x, text->pos_y, 36, WHITE);
        gl_text_render(text, 1.0);
    }
}

void continue_page(int page_num) {
    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        return;
    }
}

void animate_off_page(int page_num) {
    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        return;
    }
    
    Page *page = engine.hub->pages[page_num];
    Chroma_Rectangle *rect;
    Chroma_Text *text;

    for (int i = 0; i < page->num_rect; i++) {
        rect = &page->rect[i];
        //DrawRectangle(rect->pos_x, rect->pos_y, rect->width, rect->height, BLACK);
    }

    for (int i = 0; i < 2; i++) {
        text = &page->text[i];
        //DrawText(text->buf, text->pos_x, text->pos_y, 20, BLACK);
    }
}


