/*
 *
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "page.h"
#include <GL/gl.h>

int animate_left_to_right(uint page_num, float time) {
    ChromaRectangle *mask = &engine.hub->pages[page_num]->mask;
    ChromaRectangle *base_rect = &engine.hub->pages[page_num]->rect[0];

    mask->pos_x = base_rect->pos_x + time * base_rect->width;
    mask->pos_y = base_rect->pos_y;
    mask->width = (1.1 - time) * base_rect->width;
    mask->height = base_rect->height;
    
    return 1;
}

int animate_clock_tick(uint page_num, float time) {
    Page *page = engine.hub->pages[page_num];
    
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

    return 1;
}

int animate_none(uint page_num, float time) {
    return 1;
}
