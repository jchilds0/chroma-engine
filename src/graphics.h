/*
 * Header for page submodule
 */

#ifndef CHROMA_PAGE
#define CHROMA_PAGE

#include "chroma-engine.h"
#include "geometry.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_PAGE_SIZE     GIGABYTES((uint64_t) 8)

typedef enum {
    SET_FRAME = 0,
    USER_FRAME,
    BIND_FRAME,
} FrameType;

typedef struct {
    unsigned int      frame_num;
    unsigned int      geo_id;
    unsigned int      attr;
    FrameType         type;
    int               value;
    unsigned char     expand;
    unsigned int      bind_frame_num;
    unsigned int      bind_geo_id;
    unsigned int      bind_attr;
} Keyframe;

typedef enum {
    EVAL_LEAF = 0,
    EVAL_SINGLE_VALUE,
    EVAL_MIN_VALUE,
    EVAL_MAX_VALUE,
    EVAL_MAX_VALUE_PAD,
    EVAL_SUM_VALUE,
} NodeEval;

typedef struct Edge {
    size_t        index;
    GeometryAttr  attr;

    struct Edge   *next;
    struct Edge   *prev;
} Edge;

typedef struct Node {
    GeometryAttr  attr;
    int           value;
    NodeEval      eval;
    unsigned char evaluated;

    Edge          edge_list_head;
    Edge          edge_list_tail;

    struct Node   *next;
    struct Node   *prev;
} Node;

typedef struct {
    Arena         *arena;
    size_t        node_count;
    size_t        num_nodes;
    size_t        num_edges;

    Node          *node_list_head;
    Node          *node_list_tail;
} Graph;

typedef struct {
    int             w;
    int             h;
    unsigned char   *data;
} Image;

typedef struct {
    unsigned int    temp_id;
    Arena           arena;
    unsigned int    len_geometry;
    IGeometry       **geometry;

    unsigned int    max_keyframe;
    Graph           keyframe_graph;
} IPage;

extern IGeometry    *graphics_page_add_geometry(IPage *page, int type, int geo_id);

typedef struct {
    size_t          count;
    size_t          capacity;
    IPage           *items;

    Arena           arena;
    Image           img[MAX_ASSETS];
} IGraphics;

/* gr_hub.c */
extern void         graphics_new_graphics_hub(IGraphics *hub, int num_pages);
extern void         graphics_hub_load_example(IGraphics *hub);
extern void         graphics_free_graphics_hub(IGraphics *hub);

extern int          graphics_hub_get_page(IGraphics *hub, int temp_id);
extern IPage        *graphics_hub_new_page(IGraphics *hub, int num_geo, int max_keyframe, int temp_id);

extern uint64_t     graphics_graph_size(Graph *g);

extern Keyframe     *graphics_page_add_keyframe(IPage *page);
extern void         graphics_keyframe_set_int(Keyframe *frame, char *name, int value);
extern void         graphics_keyframe_set_attr(Keyframe *frame, char *name, char *value);
extern void         graphics_page_default_relations(IPage *page);
extern void         graphics_page_calculate_keyframes(IPage *page);
extern void         graphics_page_interpolate_geometry(IPage *page, int index, int width);

#endif // !CHROMA_PAGE
