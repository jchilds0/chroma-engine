/*
 * gr_graph.c
 */

#include "chroma-engine.h"
#include "graphics.h"
#include "graphics_internal.h"
#include "log.h"
#include <stdint.h>
#include <string.h>

void graphics_new_graph(Arena *a, Graph *g, int n) {
    g->num_nodes = n;
    g->num_edges = 0;
    g->adj_matrix = ARENA_ARRAY(a, n * n, unsigned char);
    g->value = ARENA_ARRAY(a, n, int);
    g->pad_index = ARENA_ARRAY(a, n, int);
    g->exists = ARENA_ARRAY(a, n, unsigned char);
    g->node_evals = ARENA_ARRAY(a, n, NodeEval);
}

uint64_t graphics_graph_size(Graph *g) {
    uint64_t adj_mat_size = sizeof( unsigned char ) * g->num_nodes * g->num_nodes;
    uint64_t value_size = sizeof( int ) * g->num_nodes;
    uint64_t pad_size = sizeof( int ) * g->num_nodes;
    uint64_t exists_size = sizeof( unsigned char ) * g->num_nodes;
    uint64_t eval_size = sizeof( NodeEval ) * g->num_nodes;

    return adj_mat_size + value_size + pad_size + exists_size + eval_size;
}

void graphics_graph_add_eval_node(Graph *g, int x, int pad_index, NodeEval eval) {
    if (x < 0 || x >= g->num_nodes) {
        log_file(LogError, "Graph", "Index out of range: adding eval node %d", x);
    }

    g->exists[x] = 1;
    g->pad_index[x] = pad_index;
    g->node_evals[x] = eval;
}

void graphics_graph_add_leaf_node(Graph *g, int x, int value) {
    if (x < 0 || x >= g->num_nodes) {
        log_file(LogError, "Graph", "Index out of range: adding leaf node %d", x);
    }

    g->exists[x] = 1;
    g->value[x] = value;
    g->node_evals[x] = EVAL_LEAF;
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

static int single_value(Node node) {
    int value_count = 0;
    int value = 0;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }

        value_count++;
        value = node.values[i];
    }

    if (value_count != 1) {
        log_file(LogError, "Graphics", "Node %d has %d values, expected 1", node.node_index, value_count);
    }

    return value;
}

static int min_value(Node node) {
    int value = INT_MAX;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }
        
        value = MIN(value, node.values[i]);
    }

    if (value == INT_MAX) {
        log_file(LogError, "Graphics", "Node %d missing values", node.node_index);
    }

    return value;
}

static int max_value(Node node) {
    int value = INT_MIN;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }
        
        value = MAX(value, node.values[i]);
    }

    if (value == INT_MIN) {
        log_file(LogError, "Graphics", "Node %d missing values", node.node_index);
    }

    return value;
}

static int max_value_plus_pad(Node node) {
    int value = 0;

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }

        if (i == node.pad_index) {
            continue;
        }
        
        value = MAX(value, node.values[i]);
    }

    if (!node.have_value[node.pad_index]) {
        log_file(LogError, "Graphics", "Node %d missing pad %d", node.node_index, node.pad_index);
    }

    return value + node.values[node.pad_index];
}

static int sum_value(Node node) {
    int value = 0; 

    for (int i = 0; i < node.num_values; i++) {
        if (!node.have_value[i]) {
            continue;
        }

        value += node.values[i];
    }

    return value;
}

static int graphics_graph_eval(Node n) {
    switch (n.eval) {
        case EVAL_LEAF:
            log_assert(0, "Graphics", "Cannot evaluate leaf node");
            return 0;
        case EVAL_SINGLE_VALUE:
            return single_value(n);
        case EVAL_MIN_VALUE:
            return min_value(n);
        case EVAL_MAX_VALUE:
            return max_value(n);
        case EVAL_MAX_VALUE_PAD:
            return max_value_plus_pad(n);
        case EVAL_SUM_VALUE:
            return sum_value(n);
    }
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

    if (g->node_evals[node] != EVAL_LEAF) {
        Node n = {g->num_nodes, node, g->pad_index[node], g->node_evals[node], g->value, have_value};

        g->value[node] = graphics_graph_eval(n);
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
