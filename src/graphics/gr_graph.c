/*
 * gr_graph.c
 */

#include "chroma-engine.h"
#include "geometry.h"
#include "graphics.h"
#include "graphics_internal.h"
#include "log.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void graphics_new_graph(Arena *a, Graph *g, size_t n) {
    g->arena = a;
    g->node_count = n;
    g->num_edges = 0;
    g->num_nodes = 0;

    g->node_list_head = ARENA_ARRAY(g->arena, g->node_count, Node);
    g->node_list_tail = ARENA_ARRAY(g->arena, g->node_count, Node);

    for (size_t i = 0; i < g->node_count; i++) {
        g->node_list_head[i].next = &g->node_list_tail[i];
        g->node_list_head[i].prev = NULL;
        g->node_list_tail[i].next = NULL;
        g->node_list_tail[i].prev = &g->node_list_head[i];
    }
}

uint64_t graphics_graph_size(Graph *g) {
    uint64_t node_size = (g->num_nodes + 2 * g->node_count) * sizeof( Node );
    uint64_t edge_size = g->num_edges * sizeof( Edge );
    uint64_t graph_size = sizeof( Graph );

    return node_size + edge_size + graph_size;
}

static Node *graphics_graph_create_node(Graph *g, GeometryAttr attr) {
    Node *node = ARENA_ALLOC(g->arena, Node);
    node->edge_list_head.next = &node->edge_list_tail;
    node->edge_list_head.prev = NULL;
    node->edge_list_tail.next = NULL;
    node->edge_list_tail.prev = &node->edge_list_head;

    node->attr = attr;
    g->num_nodes++;

    return node;
}

Node *graphics_graph_get_node(Graph *g, size_t index, GeometryAttr attr) {
    log_assert(index < g->node_count, "Graphics", "Index out of range " __FILE__);

    Node *head = &g->node_list_head[index];
    Node *tail = &g->node_list_tail[index];

    for (Node *node = head->next; node != tail; node = node->next) {
        if (node->attr != attr) {
            continue;
        }

        return node;
    }

    return NULL;
}

void graphics_graph_add_eval_node(Graph *g, size_t x, GeometryAttr attr, NodeEval eval) {
    if (x < 0 || x >= g->node_count) {
        log_file(LogError, "Graph", "Index out of range: adding eval node %d", x);
    }

    Node *node = graphics_graph_create_node(g, attr);
    node->eval = eval;
    node->evaluated = 0;

    INSERT_BEFORE(node, &g->node_list_tail[x]);
}

void graphics_graph_add_leaf_node(Graph *g, size_t x, GeometryAttr attr, int value) {
    if (x < 0 || x >= g->node_count) {
        log_file(LogError, "Graph", "Index out of range: adding leaf node %d", x);
    }

    Node *node = graphics_graph_create_node(g, attr);
    node->eval = EVAL_LEAF;
    node->evaluated = 1;
    node->value = value;

    INSERT_BEFORE(node, &g->node_list_tail[x]);
}

void graphics_graph_update_leaf(Graph *g, size_t x, GeometryAttr attr, int value) {
    Node *node = graphics_graph_get_node(g, x, attr);
    if (node == NULL) {
        graphics_graph_add_leaf_node(g, x, attr, value);
        return;
    }

    node->value = value;
}

void graphics_graph_add_edge(Graph *g, size_t x, GeometryAttr x_attr, size_t y, GeometryAttr y_attr) {
    if (x < 0 || x >= g->node_count) {
        log_file(LogError, "Graph", "Index out of range: adding edge from %d", x);
    }

    if (y < 0 || y >= g->node_count) {
        log_file(LogError, "Graph", "Index out of range: adding edge to %d", y);
    }

    Node *node = graphics_graph_get_node(g, x, x_attr);
    if (node == NULL) {
        log_file(LogError, "Graph", "Adding edge from node %d attr %s which does not exist", 
                 x, geometry_attr_to_char(x_attr));
    }

    Edge *edge = ARENA_ALLOC(g->arena, Edge);
    edge->index = y;
    edge->attr = y_attr;

    INSERT_BEFORE(edge, &node->edge_list_tail);
    g->num_edges++;
}

static unsigned char graphics_graph_depth_first(
    Graph *g, unsigned char *visited, unsigned char *discovered, int node) {
    return 0;
    /*
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
    */
}

unsigned char graphics_graph_is_dag(Graph *g) {
    unsigned char visited[g->node_count];
    unsigned char discovered[g->node_count];
    unsigned char is_dag = 1;

    for (int i = 0; i < g->node_count; i++) {
        visited[i] = 0;
        discovered[i] = 0;
    }

    for (int i = 0; i < g->node_count; i++) {
        if (visited[i]) {
            continue;
        }
        
        is_dag = is_dag && graphics_graph_depth_first(g, visited, discovered, i);
    }

    return is_dag;
}

static int single_value(Graph *g, Node *node) {
    if (node->edge_list_head.next == &node->edge_list_tail) {
        log_file(LogError, "Graphics", "Node %s has no values, expected 1", geometry_attr_to_char(node->attr));
    }

    Edge *edge = node->edge_list_head.next;
    Node *adj = graphics_graph_get_node(g, edge->index, edge->attr);

    if (adj == NULL) {
        log_file(LogWarn, "Graphics", "Missing node %d %s", edge->index, geometry_attr_to_char(edge->attr));
        return 0;
    }

    return adj->value;
}

static int min_value(Graph *g, Node *node) {
    int value = INT_MAX;

    for (Edge *edge = node->edge_list_head.next; edge != &node->edge_list_tail; edge = edge->next) {
        Node *adj = graphics_graph_get_node(g, edge->index, edge->attr);
        if (adj == NULL) {
            log_file(LogWarn, "Graphics", "Missing node %d %s", edge->index, geometry_attr_to_char(edge->attr));
            continue;
        }

        if (!adj->evaluated) {
            continue;
        }
        
        value = MIN(value, adj->value);
    }

    if (value == INT_MAX) {
        log_file(LogError, "Graphics", "Node %s missing values", geometry_attr_to_char(node->attr));
    }

    return value;
}

static int max_value(Graph *g, Node *node) {
    int value = INT_MIN;

    for (Edge *edge = node->edge_list_head.next; edge != &node->edge_list_tail; edge = edge->next) {
        Node *adj = graphics_graph_get_node(g, edge->index, edge->attr);
        if (adj == NULL) {
            log_file(LogWarn, "Graphics", "Missing node %d %s", edge->index, geometry_attr_to_char(edge->attr));
            continue;
        }

        if (!adj->evaluated) {
            continue;
        }
        
        value = MAX(value, adj->value);
    }

    if (value == INT_MIN) {
        log_file(LogError, "Graphics", "Node %s missing values", geometry_attr_to_char(node->attr));
    }

    return value;
}

static int sum_value(Graph *g, Node *node) {
    int value = 0; 

    for (Edge *edge = node->edge_list_head.next; edge != &node->edge_list_tail; edge = edge->next) {
        Node *adj = graphics_graph_get_node(g, edge->index, edge->attr);
        if (adj == NULL) {
            log_file(LogWarn, "Graphics", "Missing node %d %s", edge->index, geometry_attr_to_char(edge->attr));
            continue;
        }

        if (!adj->evaluated) {
            continue;
        }
        
        value += adj->value;
    }

    return value;
}

static int graphics_graph_eval(Graph *g, Node *n) {
    switch (n->eval) {
        case EVAL_LEAF:
            log_file(LogError, "Graphics", "Cannot evaluate leaf node");
            return 0;
        case EVAL_SINGLE_VALUE:
            return single_value(g, n);
        case EVAL_MIN_VALUE:
            return min_value(g, n);
        case EVAL_MAX_VALUE:
            return max_value(g, n);
        case EVAL_MAX_VALUE_PAD:
            log_file(LogError, "Graphics", "Max value plus pad not implemented");
            return 0;
        case EVAL_SUM_VALUE:
            return sum_value(g, n);
    }
}


static void graphics_graph_evaluate_node(Graph *g, Node *node) {
    if (node->evaluated || node->eval == EVAL_LEAF) {
        return;
    }

    for (Edge *edge = node->edge_list_head.next; edge != &node->edge_list_tail; edge = edge->next) {
        Node *adj = graphics_graph_get_node(g, edge->index, edge->attr);
        if (adj == NULL) {
            continue;
        }

        if (adj->evaluated) {
            continue;
        }

        graphics_graph_evaluate_node(g, adj);
    }

    node->value = graphics_graph_eval(g, node);
    node->evaluated = 1;
}

void graphics_graph_evaluate_dag(Graph *g) {
    Node *head, *tail;
    for (size_t i = 0; i < g->node_count; i++) {
        head = &g->node_list_head[i];
        tail = &g->node_list_tail[i];

        for (Node *node = head->next; node != tail; node = node->next) {
            graphics_graph_evaluate_node(g, node);
        }
    }
}
