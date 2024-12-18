/*
 * geometry_internal.h
 *
 * Header file for geometry module source code.
 * Should not be included by files outside the 
 * /geometry dir.
 *
 */

#ifndef GEOMETRY_INTERNAL
#define GEOMETRY_INTERNAL

#include "chroma-typedefs.h"
#include "geometry.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define MAX_NAME_LEN 20

/* geo_rect.c */
GeometryRect *geometry_new_rectangle(Arena *a);
void geometry_clean_rect(GeometryRect *rect);
void geometry_rectangle_set_attr(GeometryRect *rect, GeometryAttr attr, char *value);
void geometry_rectangle_get_attr(GeometryRect *rect, GeometryAttr attr, char *value);

/* geo_circle.c */
GeometryCircle *geometry_new_circle(Arena *a);
void geometry_clean_circle(GeometryCircle *circle);
void geometry_circle_set_attr(GeometryCircle *circle, GeometryAttr attr, char *value);
void geometry_circle_get_attr(GeometryCircle *circle, GeometryAttr attr, char *value);

/* geo_graph.c */
GeometryGraph *geometry_new_graph(Arena *a);
void geometry_clean_graph(GeometryGraph *g);
void geometry_graph_set_attr(GeometryGraph *g, GeometryAttr attr, char *value);
void geometry_graph_get_attr(GeometryGraph *g, GeometryAttr attr, char *value);

/* geo_text.c */
GeometryText *geometry_new_text(Arena *a);
void geometry_clean_text(GeometryText *text);
void geometry_text_set_attr(GeometryText *text, GeometryAttr attr, char *value);
void geometry_text_get_attr(GeometryText *text, GeometryAttr attr, char *value);

/* geo_image.c */
GeometryImage *geometry_new_image(Arena *a);
void geometry_clean_image(GeometryImage *image);
void geometry_image_set_attr(GeometryImage *image, GeometryAttr attr, char *value);
void geometry_image_get_attr(GeometryImage *image, GeometryAttr attr, char *value);

/* geo_poly.c */
GeometryPolygon *geometry_new_polygon(Arena *a);
void geometry_clean_polygon(GeometryPolygon *poly);
void geometry_polygon_set_attr(GeometryPolygon *poly, GeometryAttr attr, char *value);
void geometry_polygon_set_point(GeometryPolygon *poly, vec2 vec, int index);
void geometry_polygon_get_attr(GeometryPolygon *poly, GeometryAttr attr, char *value);
vec2 geometry_polygon_get_point(GeometryPolygon *poly, int index);

#endif // !GEOMETRY_INTERNAL
