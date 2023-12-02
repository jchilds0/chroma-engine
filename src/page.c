/*
 *
 */

#include "chroma-engine.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

Page *init_page(int num_rect) {
    Page *page = NEW_STRUCT(Page);
    page->num_rect = num_rect;
    page->rect = NEW_ARRAY(num_rect, Chroma_Rectangle);

    return page;
}

void free_page(Page *page) {
    free(page->rect);
    free(page);
}

void set_rect(Page *page, int rect_num, int pos_x, int pos_y, int width, int height, Color color) {
    if (!WITHIN(rect_num, 0, page->num_rect)) {
        return;
    }

    page->rect[rect_num] = (Chroma_Rectangle){pos_x, pos_y, width, height, color};
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

void animate_on_page(Graphics *hub, int page_num) {
    if (!WITHIN(page_num, 0, hub->num_pages)) {
        printf("Page out of range: %d\n", page_num);
        fflush(stdout);
        return;
    }

    hub->current_page = page_num;
    Page *page = hub->pages[page_num];
    Chroma_Rectangle *rect;
    Chroma_Text *text;

    for (int i = 0; i < page->num_rect; i++) {
        rect = &page->rect[i];
        DrawRectangle(rect->pos_x, rect->pos_y, rect->width, rect->height, rect->color);
    }


    for (int i = 0; i < 2; i++) {
        text = &page->text[i];
        text->pos_x = page->rect[0].pos_x + page->rect[0].height / 3;
        text->pos_y = page->rect[0].pos_y + (i + 1) * page->rect[0].height / 3;
        
        DrawText(text->buf, text->pos_x, text->pos_y, 36, WHITE);
    }
}

void continue_page(Graphics *hub, int page_num) {
    if (!WITHIN(page_num, 0, hub->num_pages)) {
        return;
    }
}

void animate_off_page(Graphics *hub, int page_num) {
    if (!WITHIN(page_num, 0, hub->num_pages)) {
        return;
    }
    
    Page *page = hub->pages[page_num];
    Chroma_Rectangle *rect;
    Chroma_Text *text;

    for (int i = 0; i < page->num_rect; i++) {
        rect = &page->rect[i];
        DrawRectangle(rect->pos_x, rect->pos_y, rect->width, rect->height, BLACK);
    }

    for (int i = 0; i < 2; i++) {
        text = &page->text[i];
        DrawText(text->buf, text->pos_x, text->pos_y, 20, BLACK);
    }
}


