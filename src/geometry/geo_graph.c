/*
 * Graph geometry
 *
 * Contains a collection of data points, the format
 * to draw the points in, ticks and tick size,
 * axis and axis labels.
 */

#include "chroma-engine.h"
#include "geometry_internal.h"
#include "log.h"
#include <stdio.h>
#include <string.h>


GeometryGraph *geometry_new_graph(void) {
    GeometryGraph *g = NEW_STRUCT(GeometryGraph);
    g->geo_type        = GRAPH;
    g->num_nodes       = 0;

    memset(g->nodes, 0, sizeof g->nodes);
    memset(g->x_label, '\0', GEO_BUF_SIZE);
    memset(g->y_label, '\0', GEO_BUF_SIZE);
    return g;
}

void geometry_free_graph(GeometryGraph *g) {
    free(g);
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
        case GEO_POS_X:
            sprintf(value, "%u", g->pos_x);
            break;
        case GEO_POS_Y:
            sprintf(value, "%u", g->pos_y);
            break;
        case GEO_NUM_NODE:
            sprintf(value, "%d", g->num_nodes);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a graph attr (%d)", attr);
    }
}

void geometry_graph_set_attr(GeometryGraph *graph, GeometryAttr attr, char *value) {
    int g_value = atoi(value);
    int r, g, b, a;
    int x, y, index;

    switch (attr) {
        case GEO_COLOR:
            sscanf(value, "%d %d %d %d", &r, &g, &b, &a);
            graph->color[0] = r * 1.0 / 255;
            graph->color[1] = g * 1.0 / 255;
            graph->color[2] = b * 1.0 / 255;
            graph->color[3] = a * 1.0 / 255;
            break;
        case GEO_POS_X:
            graph->pos_x = g_value;
            break;
        case GEO_POS_Y:
            graph->pos_y = g_value;
            break;
        case GEO_GRAPH_NODE:
            sscanf(value, "%d %d %d", &index, &x, &y);
            graph->nodes[index].x = x;
            graph->nodes[index].y = y;
            graph->num_nodes = MAX(index + 1, graph->num_nodes);

            break;
        case GEO_NUM_NODE:
            graph->num_nodes = g_value;
            memset(graph->nodes, 0, sizeof graph->nodes);
            break;
        default:
            log_file(LogWarn, "Geometry", "Geo attr not a graph attr (%d)", attr);
    }
}
