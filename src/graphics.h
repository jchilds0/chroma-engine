/*
 * Header for page submodule
 */

#ifndef CHROMA_PAGE
#define CHROMA_PAGE

#include "geometry.h"

typedef void IGraphics;

extern IGraphics    *graphics_new_graphics_hub(void);
extern void         graphics_hub_load_example(IGraphics *hub);
extern void         graphics_free_graphics_hub(IGraphics *hub);
extern void         graphics_hub_set_time(IGraphics *hub, float time);
extern float        graphics_hub_get_time(IGraphics *hub);

typedef void IPage;

extern IPage        *graphics_hub_get_page(IGraphics *hub, int page_num);
extern IPage        *graphics_hub_add_page(IGraphics *hub);
extern int          graphics_hub_get_current_page_num(IGraphics *hub);
extern void         graphics_hub_set_current_page_num(IGraphics *hub, int page_num);
extern void         graphics_page_update_animation(IPage *page, char *anim, float time);

extern IGeometry    *graphics_page_get_geometry(IPage *page, int geo_num);
extern IGeometry    *graphics_page_add_geometry(IPage *page, char *type);
extern int          graphics_page_num_geometry(IPage *page);

#endif // !CHROMA_PAGE
