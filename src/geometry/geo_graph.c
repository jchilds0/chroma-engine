/*
 * geo_graph.c
 *
 * Functions for the GeometryGraph struct which
 * describes a plot containing a collection of 
 * points and the type of graph.
 */

#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <string.h>


GeometryGraph *geometry_new_graph(void) {
    GeometryGraph *g = NEW_STRUCT(GeometryGraph);
    g->geo.geo_type = GRAPH;
    geometry_clean_graph(g);

    return g;
}

void geometry_free_graph(GeometryGraph *g) {
    free(g);
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
        case GEO_COLOR:
            sprintf(value, "%f %f %f %f", 
                    g->color[0], 
                    g->color[1], 
                    g->color[2], 
                    g->color[3]);
            break;
        case GEO_NUM_NODE:
            sprintf(value, "%d", g->num_nodes);
            break;
        case GEO_GRAPH_TYPE:
            sprintf(value, "%d", g->graph_type);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a graph attr (%d)", attr);
    }
}

void geometry_graph_set_attr(GeometryGraph *graph, GeometryAttr attr, char *value) {
    int g_value = atoi(value);
    int x, y;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%f %f %f %f", 
                   &graph->color[0],
                   &graph->color[1],
                   &graph->color[2],
                   &graph->color[3]);
            break;
        case GEO_GRAPH_NODE:
            sscanf(value, "%d %d", &x, &y);
            graph->nodes[graph->node_count].x = x;
            graph->nodes[graph->node_count].y = y;
            graph->node_count++;

            break;
        case GEO_NUM_NODE:
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
            log_file(LogWarn, "Geometry", "Geo attr not a graph attr (%d)", attr);
    }
}
