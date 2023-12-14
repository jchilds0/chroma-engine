/*
 *
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "page.h"
#include <GL/gl.h>

void animate_left_to_right(int page_num) {
    Chroma_Rectangle *mask = &engine.hub->pages[page_num]->mask;
    Chroma_Rectangle *base_rect = &engine.hub->pages[page_num]->rect[0];
    float time = engine.hub->pages[page_num]->mask_time;

    if (time >= 1.0) {
        return;
    }

    engine.hub->pages[page_num]->mask_time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.0); 

    mask->pos_x = base_rect->pos_x + engine.hub->pages[page_num]->mask_time * base_rect->width;
    mask->pos_y = base_rect->pos_y;
    mask->width = (1 - time) * base_rect->width;
    mask->height = base_rect->height;
}

void animate_clock_tick(int page_num) {
    float time = engine.hub->pages[page_num]->clock_time;

    if (time >= 1.0) {
        return;
    }

    engine.hub->pages[page_num]->clock_time = MIN(time + 1.0 / 100, 1.0); 

    Page *page = engine.hub->pages[page_num];
    log_file(LogMessage, "theta %f", time);
    
    GLfloat A[16] = GL_MATH_ROTATE_X(DEG_TO_RAD(time));
    GLfloat B[16] = GL_MATH_ID;

    page->text[0].do_transform[0] = 1;
    page->text[0].do_transform[4] = 1;

    page->text[1].do_transform[0] = 1;
    page->text[1].do_transform[4] = 1;

    for (int i = 0; i < 9; i++) {
        page->text[0].transform[i] = A[i]; 
        page->text[1].transform[i] = B[i];
    }
}

void animate_none(int page_num) {
}
