/*
 * graphics_internal.h
 *
 * Header file for the graphics module source code.
 * Should not be included by source files outside 
 * the /graphics dir.
 *
 */

#ifndef GRAPHICS_INTERNAL
#define GRAPHICS_INTERNAL

#include "geometry.h"

#define LOG_KEYFRAMES 1
#define KEYFRAME_TYPE_LEN 20

typedef enum {
    SET_FRAME = 0,
    USER_FRAME,
    BIND_FRAME,
} FRAME_TYPE;

typedef struct {
    unsigned int      frame_num;
    unsigned int      geo_id;
    unsigned int      attr;
    FRAME_TYPE        type;
    int               value;
    unsigned char     mask;
    unsigned char     expand;
    unsigned int      bind_frame_num;
    unsigned int      bind_geo_id;
    unsigned int      bind_attr;
} Keyframe;

typedef struct IPage {
    unsigned int      temp_id;
    unsigned int      mask_index;
    unsigned int      bg_index;

    unsigned int      len_geometry;
    IGeometry         **geometry;

    unsigned int      num_keyframe;
    unsigned int      len_keyframe;
    unsigned int      max_keyframe;
    unsigned char     *attr_keyframe;
    int               *k_value;
    Keyframe          *keyframe;
} IPage;

typedef struct {
    unsigned int      num_pages;
    unsigned int      len_pages;
    IPage             **pages;
} IGraphics;

typedef struct Node {
    int           num_values;
    int           node_index;
    int           *values;
    unsigned char *have_value;
} Node;

typedef int (*NodeEval)(Node node);

typedef struct {
    int             num_nodes;
    int             *value;
    NodeEval        *node_evals;
    unsigned char   *exists;

    unsigned char   *adj_matrix;
} Graph;

/* gr_hub.c */

// external functions
IGraphics    *graphics_new_graphics_hub(int num_pages);
void         graphics_hub_load_example(IGraphics *hub);
void         graphics_free_graphics_hub(IGraphics *hub);
IPage        *graphics_hub_add_page(IGraphics *hub, int num_geo, int num_keyframe, int temp_id);
IPage        *graphics_hub_get_page(IGraphics *hub, int page_num);

/* gr_page.c */

// external functions
IGeometry    *graphics_page_add_geometry(IPage *page, int id, int type);
IGeometry    *graphics_page_get_geometry(IPage *page, int geo_num);
int          graphics_page_num_geometry(IPage *page);

// internal functions
IPage        *graphics_new_page(int num_geo, int num_keyframe);
void         graphics_free_page(IPage *);

/* gr_animation.c */
int          graphics_animate_left_to_right(IPage *page, float time);
int          graphics_animate_right_to_left(IPage *page, float time);
int          graphics_animate_up(IPage *page, float time);
int          graphics_animate_none(IPage *page, float time);

/* gr_keyframe.c */
int          graphics_keyframe_interpolate_int(int v_start, int v_end, int index, int width);

/* gr_graph.c */
Graph        *graphics_new_graph(int n);
void         graphics_graph_add_node(Graph *g, int x, int value, NodeEval f);
void         graphics_graph_add_edge(Graph *g, int x, int y);
void         graphics_graph_free_graph(Graph *g);
unsigned char graphics_graph_is_dag(Graph *g);
void graphics_graph_evaluate_dag(Graph *g);

#endif // !GRAPHICS_INTERNAL
