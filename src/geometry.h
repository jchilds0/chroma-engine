/*
 *
 *
 */

#ifndef CHROMA_GEOMETRY
#define CHROMA_GEOMETRY

#include <GL/glew.h>
#include <GL/gl.h>

typedef void IGeometry;

extern IGeometry *geometry_create_geometry(char *);
extern void geometry_free_geometry(IGeometry *geo);

extern void geometry_set_attr(IGeometry *geo, char *attr, char *value);
extern void geometry_set_int_attr(IGeometry *geo, char *attr, int value);
extern void geometry_get_attr(IGeometry *geo, char *attr, char *value);
extern int geometry_get_int_attr(IGeometry *geo, char *attr);

extern void geometry_gl_init(void);
extern void geometry_draw_geometry(IGeometry *geo);


#endif // !CHROMA_GEOMETRY
