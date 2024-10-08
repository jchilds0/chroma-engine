/*
 * gr_graph.c
 */

#include "chroma-engine.h"
#include "graphics_internal.h"
#include "log.h"
#include <stdlib.h>

void graphics_new_graph(Arena *a, Graph *g, int n) {
    g->num_nodes = n;
    g->num_edges = 0;
    g->adj_matrix = ARENA_ARRAY(a, n * n, unsigned char);
    g->value = ARENA_ARRAY(a, n, int);
    g->pad_index = ARENA_ARRAY(a, n, int);
    g->exists = ARENA_ARRAY(a, n, unsigned char);
    g->node_evals = ARENA_ARRAY(a, n, NodeEval);
}

void graphics_graph_add_eval_node(Graph *g, int x, int pad_index, NodeEval f) {
    if (x < 0 || x >= g->num_nodes) {
        log_file(LogError, "Graph", "Index out of range: adding eval node %d", x);
    }

    g->exists[x] = 1;
    g->pad_index[x] = pad_index;
    g->node_evals[x] = f;
}

void graphics_graph_add_leaf_node(Graph *g, int x, int value) {
    if (x < 0 || x >= g->num_nodes) {
        log_file(LogError, "Graph", "Index out of range: adding leaf node %d", x);
    }

    g->exists[x] = 1;
    g->value[x] = value;
    g->node_evals[x] = NULL;
}

void graphics_graph_add_edge(Graph *g, int x, int y) {
    if (x < 0 || x >= g->num_nodes) {
        log_file(LogError, "Graph", "Index out of range: adding edge from %d", x);
    }

    if (y < 0 || y >= g->num_nodes) {
        log_file(LogError, "Graph", "Index out of range: adding edge to %d", y);
    }

    g->adj_matrix[x * g->num_nodes + y] = 1;
    g->num_edges++;
}

static unsigned char graphics_graph_depth_first(
    Graph *g, unsigned char *visited, unsigned char *discovered, int node) {
    unsigned char is_dag = 1;

    discovered[node] = 1;

    for (int i = 0; i < g->num_nodes; i++) {
        if (!g->adj_matrix[node * g->num_nodes + i]) {
            continue;
        }

        if (visited[i] || i == node) {
            continue;
        }

        if (discovered[i]) {
            is_dag = 0;
            continue;
        }

        is_dag = is_dag && graphics_graph_depth_first(g, visited, discovered, i);
    }


    discovered[node] = 0;
    visited[node] = 1;
    return is_dag;
}

unsigned char graphics_graph_is_dag(Graph *g) {
    unsigned char visited[g->num_nodes];
    unsigned char discovered[g->num_nodes];
    unsigned char is_dag = 1;

    for (int i = 0; i < g->num_nodes; i++) {
        visited[i] = 0;
        discovered[i] = 0;
    }

    for (int i = 0; i < g->num_nodes; i++) {
        if (visited[i]) {
            continue;
        }
        
        is_dag = is_dag && graphics_graph_depth_first(g, visited, discovered, i);
    }

    return is_dag;
}

static void graphics_graph_evaluate_node(Graph *g, unsigned char *eval, int node) {
    unsigned char *have_value = &g->adj_matrix[node * g->num_nodes];

    for (int i = 0; i < g->num_nodes; i++) {
        if (!have_value[i] || i == node) {
            continue;
        }

        if (eval[i]) {
            continue;
        }

        graphics_graph_evaluate_node(g, eval, i);
    }

    if (g->node_evals[node] != NULL) {
        Node n = {g->num_nodes, node, g->pad_index[node], g->value, have_value};

        g->value[node] = (g->node_evals[node])(n);
    }

    eval[node] = 1;
}

void graphics_graph_evaluate_dag(Graph *g) {
    unsigned char eval[g->num_nodes];

    for (int i = 0; i < g->num_nodes; i++) {
        eval[i] = 0;
    }

    for (int i = 0; i < g->num_nodes; i++) {
        if (eval[i]) {
            continue;
        }

        if (!g->exists[i]) {
            continue;
        }

        graphics_graph_evaluate_node(g, eval, i);
    }
}
