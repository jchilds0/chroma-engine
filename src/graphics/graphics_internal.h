/*
 * graphics_internal.h
 *
 * Header file for the graphics module source code.
 * Should not be included by source files outside 
 * the /graphics dir.
 *
 */

#ifndef PAGE_INTERNAL
#define PAGE_INTERNAL

#include "chroma-engine.h"
#include "geometry.h"

typedef enum {
    POS_X = 0,
    POS_Y,
    REL_X,
    REL_Y,
    PARENT,
    WIDTH,
    HEIGHT,
    ROUNDING,
    INNER_RADIUS,
    OUTER_RADIUS,
    START_ANGLE,
    END_ANGLE,
    NUM_ATTR
} FRAME_ATTR;

char *ATTR[] = { 
    "pos_x", 
    "pos_y", 
    "rel_x", 
    "rel_y", 
    "parent", 
    "width", 
    "height", 
    "rounding", 
    "inner_radius",
    "outer_radius",
    "start_angle",
    "end_angle",
};

typedef enum {
    SET_VALUE = 0,
    USER_VALUE,
    BIND_VALUE,
} FRAME_TYPE;

typedef struct {
    unsigned int      frame_num;
    unsigned int      geo_id;
    char              attr[GEO_BUF_SIZE];
    FRAME_TYPE        type;
    int               value;
    unsigned int      bind_frame_num;
    unsigned int      bind_geo_id;
    char              bind_attr[GEO_BUF_SIZE];
} Keyframe;

typedef struct IPage {
    unsigned int      temp_id;
    unsigned int      mask_index;
    unsigned int      bg_index;

    unsigned int      num_geometry;
    unsigned int      len_geometry;
    IGeometry         **geometry;

    unsigned int      num_keyframe;
    unsigned int      len_keyframe;
    unsigned int      max_keyframe;
    int               *k_value;
    Keyframe          *keyframe;
} IPage;

typedef struct {
    unsigned int      num_pages;
    unsigned int      len_pages;
    unsigned int      current_page[CHROMA_LAYERS];
    float             time[CHROMA_LAYERS];
    IPage             **pages;
} IGraphics;

/* gr_hub.c */

// external functions
IGraphics    *graphics_new_graphics_hub(int num_pages);
void         graphics_hub_load_example(IGraphics *hub);
void         graphics_free_graphics_hub(IGraphics *hub);
IPage        *graphics_hub_add_page(IGraphics *hub, int num_geo, int num_keyframe, int temp_id);
IPage        *graphics_hub_get_page(IGraphics *hub, int page_num);
void         graphics_hub_set_time(IGraphics *hub, float time, int layer);
float        graphics_hub_get_time(IGraphics *hub, int layer);
int          graphics_hub_get_current_page_num(IGraphics *hub, int layer);
void         graphics_hub_set_current_page_num(IGraphics *hub, int page_num, int layer);

/* gr_page.c */

// external functions
IGeometry    *graphics_page_add_geometry(IPage *page, int id, char *type);
IGeometry    *graphics_page_get_geometry(IPage *page, int geo_num);
int          graphics_page_num_geometry(IPage *page);

// internal functions
IPage        *graphics_new_page(int num_geo, int num_keyframe);
void         graphics_free_page(IPage *);

/* gr_animation.c */
int graphics_animate_left_to_right(IPage *page, float time);
int graphics_animate_right_to_left(IPage *page, float time);
int graphics_animate_up(IPage *page, float time);
int graphics_animate_none(IPage *page, float time);

#endif // !PAGE_INTERNAL
