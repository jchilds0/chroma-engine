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

#include "geometry.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define MAX_NAME_LEN 20

/* geo_rect.c */
GeometryRect *geometry_new_rectangle(void);
void geometry_free_rectangle(GeometryRect *rect);
void geometry_clean_rect(GeometryRect *rect);
void geometry_rectangle_set_attr(GeometryRect *rect, GeometryAttr attr, char *value);
void geometry_rectangle_get_attr(GeometryRect *rect, GeometryAttr attr, char *value);

/* geo_circle.c */
GeometryCircle *geometry_new_circle(void);
void geometry_free_circle(GeometryCircle *circle);
void geometry_clean_circle(GeometryCircle *circle);
void geometry_circle_set_attr(GeometryCircle *circle, GeometryAttr attr, char *value);
void geometry_circle_get_attr(GeometryCircle *circle, GeometryAttr attr, char *value);

/* geo_graph.c */
GeometryGraph *geometry_new_graph(void);
void geometry_free_graph(GeometryGraph *g);
void geometry_clean_graph(GeometryGraph *g);
void geometry_graph_set_attr(GeometryGraph *g, GeometryAttr attr, char *value);
void geometry_graph_get_attr(GeometryGraph *g, GeometryAttr attr, char *value);

/* geo_text.c */
GeometryText *geometry_new_text(void);
void geometry_free_text(GeometryText *text);
void geometry_clean_text(GeometryText *text);
void geometry_text_set_attr(GeometryText *text, GeometryAttr attr, char *value);
void geometry_text_get_attr(GeometryText *text, GeometryAttr attr, char *value);

/* geo_image.c */
GeometryImage *geometry_new_image(void);
void geometry_free_image(GeometryImage *image);
void geometry_clean_image(GeometryImage *image);
void geometry_image_set_attr(GeometryImage *image, GeometryAttr attr, char *value);
void geometry_image_get_attr(GeometryImage *image, GeometryAttr attr, char *value);

#endif // !GEOMETRY_INTERNAL
