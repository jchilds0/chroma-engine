/*
 *
 */

#ifndef GEOMETRY_INTERNAL
#define GEOMETRY_INTERNAL

#include <GL/glew.h>
#include <GL/gl.h>

#define GEO_BUF_SIZE          100

typedef enum {
    GEO_COLOR,
    GEO_POS_X,
    GEO_POS_Y,
    GEO_WIDTH,
    GEO_HEIGHT,
    GEO_RADIUS,
    GEO_CENTER_X,
    GEO_CENTER_Y,
    GEO_INNER_RADIUS,
    GEO_OUTER_RADIUS,
    GEO_TEXT,
    GEO_SCALE,
} GeometryAttr;

typedef enum {
    RECT,
    CIRCLE,
    ANNULUS,
    TEXT,
} GeometryType;

typedef struct {
    GeometryType      geo_type;
} IGeometry;

/* g_rect.c */
typedef struct {
    GeometryType      geo_type;
    int               pos_x;
    int               pos_y;
    int               width;
    int               height;
    GLfloat           color[4];
} GeometryRect;

GeometryRect *geometry_new_rectangle(void);
void geometry_rectangle_set_attr(GeometryRect *rect, GeometryAttr attr, char *value);
void geometry_rectangle_get_attr(GeometryRect *rect, GeometryAttr attr, char *value);

void geometry_rectangle_init_buffers(void);
void geometry_rectangle_init_shader(void);
void geometry_draw_rectangle(GeometryRect *rect);

/* g_circle.c */
typedef struct {
    GeometryType      geo_type;
    int               center_x;
    int               center_y;
    int               radius;
    GLfloat           color[4];
} GeometryCircle;

GeometryCircle *geometry_new_circle(void);
void geometry_circle_set_attr(GeometryCircle *circle, GeometryAttr attr, char *value);
void geometry_circle_get_attr(GeometryCircle *circle, GeometryAttr attr, char *value);

void geometry_circle_init_buffers(void);
void geometry_circle_init_shader(void);
void geometry_draw_circle(GeometryCircle *circle);

/* g_annulus.c */
typedef struct {
    GeometryType      geo_type;
    int               center_x;
    int               center_y;
    int               inner_radius;
    int               outer_radius;
    GLfloat           color[4];
} GeometryAnnulus;

GeometryAnnulus *geometry_new_annulus(void);
void geometry_annulus_set_attr(GeometryAnnulus *annulus, GeometryAttr attr, char *value);
void geometry_annulus_get_attr(GeometryAnnulus *annulus, GeometryAttr attr, char *value);

void geometry_annulus_init_buffers(void);
void geometry_annulus_init_shader(void);
void geometry_draw_annulus(GeometryAnnulus *annulus);

/* g_text.c */
typedef struct {
    GeometryType      geo_type;
    int               pos_x;
    int               pos_y;
    float             scale;
    char              buf[GEO_BUF_SIZE];
    GLfloat           color[4];
} GeometryText;

GeometryText *geometry_new_text(void);
void geometry_text_set_attr(GeometryText *text, GeometryAttr attr, char *value);
void geometry_text_get_attr(GeometryText *text, GeometryAttr attr, char *value);

void geometry_text_init_buffers(void);
void geometry_text_init_shader(void);
void geometry_text_cache_characters(void);
void geometry_draw_text(GeometryText *text);

#endif // !GEOMETRY_INTERNAL
