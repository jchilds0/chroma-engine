/*
 * Header for page submodule
 */

#ifndef CHROMA_PAGE
#define CHROMA_PAGE

#include "geometry.h"

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

typedef struct GeometryNode {
    IGeometry *geo;
    struct GeometryNode *next;
    struct GeometryNode *prev;
} GeometryNode;

typedef struct KeyframeNode {
    Keyframe *frame;
    struct KeyframeNode *next;
    struct KeyframeNode *prev;
} KeyframeNode;

typedef struct {
    int           num_values;
    int           node_index;
    int           pad_index;
    int           *values;
    unsigned char *have_value;
} Node;

typedef int (*NodeEval)(Node node);

typedef struct {
    int             num_nodes;
    int             *value;
    int             *pad_index;
    NodeEval        *node_evals;
    unsigned char   *exists;

    unsigned char   *adj_matrix;
} Graph;

typedef struct {
    unsigned int      temp_id;

    GeometryNode      geo_head;
    GeometryNode      geo_tail;
    unsigned int      len_geometry;
    IGeometry         **geometry;

    KeyframeNode      frame_head;
    KeyframeNode      frame_tail;
    unsigned int      max_keyframe;
    Graph             *keyframe_graph;
} IPage;

extern IGeometry    *graphics_page_add_geometry(IPage *page, int type);

typedef struct {
    unsigned int      num_pages;
    unsigned int      len_pages;
    IPage             **pages;
} IGraphics;

extern IGraphics    *graphics_new_graphics_hub(int num_pages);
extern void         graphics_hub_load_example(IGraphics *hub);
extern void         graphics_free_graphics_hub(IGraphics *hub);
extern void         graphics_hub_free_page(IGraphics *hub, int page_num);

extern IPage        *graphics_hub_get_page(IGraphics *hub, int page_num);
extern void         graphics_hub_add_page(IGraphics *hub, IPage *page);

extern Keyframe     *graphics_page_add_keyframe(IPage *page);
extern void         graphics_keyframe_set_int(Keyframe *frame, char *name, int value);
extern void         graphics_keyframe_set_attr(Keyframe *frame, char *name, char *value);
extern void         graphics_page_default_relations(IPage *page);
extern void         graphics_page_calculate_keyframes(IPage *page);
extern void         graphics_page_interpolate_geometry(IPage *page, int index, int width);

#endif // !CHROMA_PAGE
