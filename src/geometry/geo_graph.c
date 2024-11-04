/*
 * geo_graph.c
 *
 * Functions for the GeometryGraph struct which
 * describes a plot containing a collection of 
 * points and the type of graph.
 */

#include "geometry_internal.h"
#include <stdio.h>

GeometryGraph *geometry_new_graph(Arena *a) {
    GeometryGraph *g = ARENA_ALLOC(a, GeometryGraph);
    g->geo.geo_type = GRAPH;
    geometry_clean_graph(g);

    return g;
}

void geometry_clean_graph(GeometryGraph *g) {
    g->num_nodes       = 0;
    g->node_count      = 0;
    g->graph_type      = LINE;

    memset(g->nodes, 0, sizeof g->nodes);
    memset(g->x_label, '\0', GEO_BUF_SIZE);
    memset(g->y_label, '\0', GEO_BUF_SIZE);
}

void geometry_graph_get_attr(GeometryGraph *g, GeometryAttr attr, char *value) {
    switch (attr) {
        case GEO_COLOR_R:
            sprintf(value, "%f", g->color.x);
            break;
        case GEO_COLOR_G:
            sprintf(value, "%f", g->color.y);
            break;
        case GEO_COLOR_B:
            sprintf(value, "%f", g->color.z);
            break;
        case GEO_COLOR_A:
            sprintf(value, "%f", g->color.w);
            break;
        case GEO_NUM_POINTS:
            sprintf(value, "%d", g->num_nodes);
            break;
        case GEO_GRAPH_TYPE:
            sprintf(value, "%d", g->graph_type);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a graph attr: %s", geometry_attr_to_char(attr));
    }
}

void geometry_graph_set_attr(GeometryGraph *graph, GeometryAttr attr, char *value) {
    int g_value = atoi(value);
    int x, y;

    switch (attr) {
        case GEO_COLOR_R:
            graph->color.x = (float) g_value;
            break;
        case GEO_COLOR_G:
            graph->color.y = (float) g_value;
            break;
        case GEO_COLOR_B:
            graph->color.z = (float) g_value;
            break;
        case GEO_COLOR_A:
            graph->color.w = (float) g_value;
            break;
        case GEO_POINT:
            sscanf(value, "%d %d", &x, &y);
            graph->nodes[graph->node_count].x = x;
            graph->nodes[graph->node_count].y = y;
            graph->node_count++;

            break;
        case GEO_NUM_POINTS:
            graph->num_nodes = g_value;
            graph->node_count = 0;
            memset(graph->nodes, 0, sizeof graph->nodes);
            break;
        case GEO_GRAPH_TYPE:
            if (strncmp(value, "line", 4) == 0) {
                graph->graph_type = LINE;
            } else if (strncmp(value, "bezier", 6) == 0) {
                graph->graph_type = BEZIER;
            } else if (strncmp(value, "bar", 3) == 0) {
                graph->graph_type = BAR;
            } else if (strncmp(value, "point", 5) == 0) {
                graph->graph_type = POINT;
            } else if (strncmp(value, "step", 4) == 0) {
                graph->graph_type = STEP;
            } else {
                log_file(LogWarn, "Geometry", "Unknown graph type (%s)", value);
            }
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a graph attr: %s", geometry_attr_to_char(attr));
    }
}
