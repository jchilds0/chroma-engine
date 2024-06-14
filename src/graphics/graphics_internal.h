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

#include "graphics.h"

#define LOG_KEYFRAMES         0
#define KEYFRAME_TYPE_LEN     20


/* gr_page.c */
IPage        *graphics_new_page(int num_geo, int num_keyframe);
void         graphics_free_page(IPage *);

/* gr_keyframe.c */
int          graphics_keyframe_interpolate_int(int v_start, int v_end, int index, int width);

/* gr_graph.c */
Graph         *graphics_new_graph(int n);
void          graphics_graph_add_eval_node(Graph *g, int x, int pad_index, NodeEval f);
void          graphics_graph_add_leaf_node(Graph *g, int x, int value);
void          graphics_graph_add_edge(Graph *g, int x, int y);
void          graphics_graph_free_graph(Graph *g);
unsigned char graphics_graph_is_dag(Graph *g);
void          graphics_graph_evaluate_dag(Graph *g);

#endif // !GRAPHICS_INTERNAL
