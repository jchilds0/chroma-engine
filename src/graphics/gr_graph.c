/*
 * gr_graph.c
 */

#include "chroma-engine.h"
#include "graphics_internal.h"
#include <stdlib.h>

Graph *graphics_new_graph(int n) {
    Graph *g = NEW_STRUCT(Graph);

    g->num_nodes = n;
    g->num_edges = 0;
    g->adj_matrix = NEW_ARRAY(n * n, unsigned char);
    g->value = NEW_ARRAY(n, int);
    g->pad_index = NEW_ARRAY(n, int);
    g->exists = NEW_ARRAY(n, unsigned char);
    g->node_evals = NEW_ARRAY(n, NodeEval);

    for (int i = 0; i < n * n; i++) {
        g->adj_matrix[i] = 0;
    }

    for (int i = 0; i < n; i++) {
        g->exists[i] = 0;
        g->node_evals[i] = NULL;
        g->value[i] = 0;
        g->pad_index[i] = 0;
    }

    return g;
}

void graphics_graph_add_eval_node(Graph *g, int x, int pad_index, NodeEval f) {
    g->exists[x] = 1;
    g->pad_index[x] = pad_index;
    g->node_evals[x] = f;
}

void graphics_graph_add_leaf_node(Graph *g, int x, int value) {
    g->exists[x] = 1;
    g->value[x] = value;
    g->node_evals[x] = NULL;
}

void graphics_graph_add_edge(Graph *g, int x, int y) {
    g->adj_matrix[x * g->num_nodes + y] = 1;
    g->num_edges++;
}

void graphics_graph_free_graph(Graph *g) {
    free(g->adj_matrix);
    free(g->exists);
    free(g->node_evals);
    free(g->value);
    free(g->pad_index);

    free(g);
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
    unsigned char *visited = NEW_ARRAY(g->num_nodes, unsigned char);
    unsigned char *discovered = NEW_ARRAY(g->num_nodes, unsigned char);
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

    free(visited);
    free(discovered);

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
    unsigned char *eval = NEW_ARRAY(g->num_nodes, unsigned char);

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

    free(eval);
}
