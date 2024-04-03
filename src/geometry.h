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
    GRAPH,
    TEXT,
    IMAGE,
} GeometryType;

typedef struct {
    GeometryType      geo_type;
    int               parent;
    vec2              pos;
    vec2              rel;
} IGeometry;

typedef struct {
    IGeometry         geo;
    int               width;
    int               height;
    int               rounding;
    GLfloat           color[4];
} GeometryRect;

typedef struct {
    IGeometry         geo;
    int               inner_radius;
    int               outer_radius;
    float             start_angle;
    float             end_angle;
    GLfloat           color[4];
} GeometryCircle;

typedef enum {
    LINE,
    BEZIER,
    BAR,
    POINT,
    STEP,
} GraphType;

typedef struct {
    IGeometry         geo;
    GraphType         graph_type;
    char              x_label[GEO_BUF_SIZE];
    char              y_label[GEO_BUF_SIZE];
    int               node_count;
    int               num_nodes;
    GLfloat           color[4];
    vec2              nodes[MAX_NODES];
} GeometryGraph;

typedef struct {
    IGeometry         geo;
    float             scale;
    char              buf[GEO_BUF_SIZE];
    GLfloat           color[4];
} GeometryText;

typedef struct {
    IGeometry         geo;
    float             scale;
    int               w;
    int               h;
    int               image_id;
    int               cur_image_id;
    unsigned char     *data;
} GeometryImage;

extern IGeometry *geometry_create_geometry(char *);
extern void   geometry_free_geometry(IGeometry *geo);

extern void   geometry_set_attr(IGeometry *geo, char *attr, char *value);
extern void   geometry_set_int_attr(IGeometry *geo, char *attr, int value);
extern void   geometry_set_float_attr(IGeometry *geo, char *attr, float value);

extern void   geometry_get_attr(IGeometry *geo, char *attr, char *value);
extern int    geometry_get_int_attr(IGeometry *geo, char *attr);
extern float  geometry_get_float_attr(IGeometry *geo, char *attr);

#endif // !CHROMA_GEOMETRY
