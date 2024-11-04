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
#include "graphics.h"
#include <stddef.h>

#define LOG_KEYFRAMES         0

/* gr_page.c */
void         graphics_init_page(IPage *, int num_geo, int max_keyframe);
void         graphics_page_clear(IPage *);
void         graphics_page_generate(IPage *);
int          graphics_page_free_page(IPage *);

/* gr_keyframe.c */
FrameType    graphics_keyframe_type(char *name);   
float       graphics_keyframe_interpolate(float v_start, float v_end, int index, int width);

/* gr_graph.c */
void          graphics_new_graph(Arena *a, Graph *g, size_t n);
Node          *graphics_graph_get_node(Graph *g, size_t index, GeometryAttr attr);
void          graphics_graph_add_eval_node(Graph *g, size_t x, GeometryAttr attr, NodeEval f);
void          graphics_graph_add_leaf_node(Graph *g, size_t x, GeometryAttr attr, float value);
Edge          *graphics_graph_add_edge(Graph *g, size_t x, GeometryAttr x_attr, 
                                      size_t y, GeometryAttr y_attr);
void          graphics_graph_evaluate_dag(Graph *g);

#endif // !GRAPHICS_INTERNAL
