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

#define PAGE_MIN_GEOMETRY     20
#define PAGE_MIN_GEOMETRY     20

/* gr_page.c */
IPage        *graphics_new_page(void);
void         graphics_free_page(IPage *);
void         graphics_page_generate(IPage *);

/* gr_keyframe.c */
FrameType    graphics_keyframe_type(char *name);   
int          graphics_keyframe_interpolate_int(int v_start, int v_end, int index, int width);
void         graphics_page_gen_frame(IPage *page, Keyframe frame);

/* gr_graph.c */
Graph         *graphics_new_graph(int n);
void          graphics_graph_add_eval_node(Graph *g, int x, int pad_index, NodeEval f);
void          graphics_graph_add_leaf_node(Graph *g, int x, int value);
void          graphics_graph_add_edge(Graph *g, int x, int y);
void          graphics_graph_free_graph(Graph *g);
unsigned char graphics_graph_is_dag(Graph *g);
void          graphics_graph_evaluate_dag(Graph *g);

#endif // !GRAPHICS_INTERNAL
