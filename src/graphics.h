/*
 * Header for page submodule
 */

#ifndef CHROMA_PAGE
#define CHROMA_PAGE

#include <stdio.h>
#include "geometry.h"

typedef void IGraphics;

extern IGraphics    *graphics_new_graphics_hub(void);
extern void         graphics_load_from_file(IGraphics *hub, FILE *file);
extern void         graphics_free_graphics_hub(IGraphics *hub);
extern void         graphics_hub_set_time(IGraphics *hub, float time);
extern float        graphics_hub_get_time(IGraphics *hub);

extern void         graphics_page_update_on(IGraphics *hub, int page_num);
extern void         graphics_page_update_cont(IGraphics *hub, int page_num);
extern void         graphics_page_update_off(IGraphics *hub, int page_num);

typedef void IPage;

extern IPage        *graphics_hub_get_page(IGraphics *hub, int page_num);
extern int          graphics_hub_get_current_page_num(IGraphics *hub);
extern void         graphics_hub_set_current_page(IGraphics *hub, int page_num);
extern IGeometry    *graphics_page_get_geometry(IPage *page, int geo_num);
extern int          graphics_page_num_geometry(IPage *page);

#endif // !CHROMA_PAGE
