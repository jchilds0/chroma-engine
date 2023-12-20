/*
 * Header file for the geometry module.
 *
 * Contains typdefs and declarations for the different 
 * viz geometries. 
 *
 * Each geometry should start with the same structure 
 * as the IGeometry. This struct is used by the page to
 * store each geometry without knowing the internals. 
 *
 * A specific geometry is then access by casting an 
 * IGeometry pointer to the appropriate concrete type.
 * It is important the geo_type attr is NOT CHANGED
 * as it is used to decide the type to cast to.
 *
 * Each geometry type implements a get and set attr 
 * method. These are primarily used by the parser
 * for a simple API to update values set by the user.
 *
 * The Open GL renderer either uses the get api or 
 * pointer casts depending on what is convenient.
 * Ideally the API is used, but for some data such
 * as arrays (i.e. the graph nodes) it makes more 
 * sense to simply cast the IGeometry pointer and 
 * access directly
 *
 */

#ifndef CHROMA_GEOMETRY
#define CHROMA_GEOMETRY

#include "gl_math.h"
#include <GL/glew.h>
#include <GL/gl.h>

#define GEO_BUF_SIZE          100
#define MAX_NODES             100

typedef enum {
    RECT,
    CIRCLE,
    ANNULUS,
    GRAPH,
    TEXT,
} GeometryType;

typedef struct {
    GeometryType      geo_type;
} IGeometry;

typedef struct {
    GeometryType      geo_type;
    int               pos_x;
    int               pos_y;
    int               width;
    int               height;
    GLfloat           color[4];
} GeometryRect;

typedef struct {
    GeometryType      geo_type;
    int               center_x;
    int               center_y;
    int               radius;
    GLfloat           color[4];
} GeometryCircle;

typedef struct {
    GeometryType      geo_type;
    int               center_x;
    int               center_y;
    int               inner_radius;
    int               outer_radius;
    GLfloat           color[4];
} GeometryAnnulus;

typedef struct {
    GeometryType      geo_type;
    int               pos_x;
    int               pos_y;
    char              x_label[GEO_BUF_SIZE];
    char              y_label[GEO_BUF_SIZE];
    int               num_nodes;
    GLfloat           color[4];
    vec2              nodes[MAX_NODES];
} GeometryGraph;

typedef struct {
    GeometryType      geo_type;
    int               pos_x;
    int               pos_y;
    float             scale;
    char              buf[GEO_BUF_SIZE];
    GLfloat           color[4];
} GeometryText;

extern IGeometry *geometry_create_geometry(char *);
extern void geometry_free_geometry(IGeometry *geo);

extern void geometry_set_attr(IGeometry *geo, char *attr, char *value);
extern void geometry_set_int_attr(IGeometry *geo, char *attr, int value);
extern void geometry_get_attr(IGeometry *geo, char *attr, char *value);
extern int geometry_get_int_attr(IGeometry *geo, char *attr);

extern void geometry_gl_init(void);
extern void geometry_draw_geometry(IGeometry *geo);

#endif // !CHROMA_GEOMETRY
