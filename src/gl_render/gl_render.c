/*
 * gl_render.c 
 *
 * Callbacks for GTK_GL_AREA used in preview.c and engine.c
 */

#include "gl_render_internal.h"
#include "chroma-typedefs.h"

#define ANIM_LENGTH     120

int action[] = {BLANK, BLANK, BLANK, BLANK, BLANK};
int page_num[] = {-1, -1, -1, -1, -1};
int current_page[] = {-1, -1, -1, -1, -1};
int frame_num[] = {0, 0, 0, 0, 0};
float frame_time[] = {0.0, 0.0, 0.0, 0.0, 0.0};

Renderer r;

/* Create and compile a shader */
GLuint gl_renderer_create_shader(int type, const char *src) {
    GLuint shader;
    int status;

    shader = glCreateShader (type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        int log_len;
        char *buffer;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

        buffer = g_malloc(log_len + 1);
        glGetShaderInfoLog(shader, log_len, NULL, buffer);

        g_warning("Compile failure in %s shader:\n%s",
                    type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                    buffer);

        g_free(buffer);

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

GLuint gl_renderer_create_program(GLuint vertex, GLuint fragment) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        int log_len;
        char *buffer;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

        buffer = g_malloc(log_len + 1);
        glGetProgramInfoLog(program, log_len, NULL, buffer);

        g_warning("Linking failure:\n%s", buffer);

        g_free(buffer);

        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

void gl_realize(GtkWidget *widget) {
    GdkFrameClock *frame_clock;
    gtk_gl_area_make_current(GTK_GL_AREA(widget));
    glewInit();

    // tie render to frame clock
    frame_clock = gtk_widget_get_frame_clock(widget);
    g_signal_connect_swapped(frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), widget);
    gdk_frame_clock_begin_updating(frame_clock);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

    gl_graph_init_buffers();
    gl_graph_init_shaders();

    gl_text_init_buffers();
    gl_text_init_shaders();
    gl_text_cache_characters();

    gl_image_init_buffers();
    gl_image_init_shaders();

    gl_renderer_triangle_init(&r, glrender_vs_glsl, glrender_fs_glsl);                             
}

void gl_renderer_set_scale(GLuint program) {
    glUseProgram(program);

    GLfloat view[] = GL_MATH_TRANSLATE(0, 0, -1);
    GLfloat model[] = GL_MATH_ID;
    GLfloat ortho[] = GL_MATH_ORTHO(0.0, 1920.0, 0.0, 1080.0, -1.0, 1.0);

    uint model_loc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(model_loc, 1, GL_TRUE, model);

    uint view_loc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(view_loc, 1, GL_TRUE, view);

    uint ortho_loc = glGetUniformLocation(program, "ortho");
    glUniformMatrix4fv(ortho_loc, 1, GL_TRUE, ortho);
}

static float gl_bezier_time_step(float time, float start, float end, int order) {
    if (order == 1) {
        return (1 - time) * start + time * end;
    }

    float p1 = gl_bezier_time_step(time, start, end, order - 1);
    float p2 = gl_bezier_time_step(time, end, end, order - 1);

    return (1 - time) * p1 + time * p2;
}

static void gl_render_draw_geometry(Renderer *r, IGeometry *geo) {
    switch (geo->geo_type) {
        case RECT:
            gl_draw_rectangle(r, (GeometryRect *)geo);
            break;

        case CIRCLE:
            gl_draw_circle(r, (GeometryCircle *)geo);
            break;

        case TEXT:
            gl_draw_text(geo);
            break;

        case GRAPH:
            gl_draw_graph(geo);
            break;

        case IMAGE:
            gl_draw_image(geo);
            break;

        case POLYGON:
            gl_draw_polygon(r, (GeometryPolygon *)geo);
            break;

        default:
            log_file(LogWarn, "GL Renderer", "Unknown geo type (%d)", geo->geo_type);
    }
}

static int gl_render_has_child(IPage *page, int geo_num) {
    IGeometry *geo;
    int retval = 0;

    for (size_t i = 0; i < page->len_geometry; i++) {
        geo = page->geometry[i];
        if (geo == NULL) {
            continue;
        }

        if (geo->parent_id != geo_num) {
            continue;
        }

        retval = 1;
    }

    return retval;
}

static void gl_render_clear_bit(IPage *page, uint depth) {
    GeometryRect rect = {{RECT, 0, 0, {0, 0}, {0, 0}, 0}, 1920, 1080, 0, {0, 0, 0, 0}};

    gl_renderer_mask(&r, RENDER_MASK_CLEAR, depth);

    gl_draw_rectangle(&r, &rect);

    gl_renderer_use(&r);
    gl_renderer_draw(&r);
}

static void gl_render_draw_heirachy(IPage *page, IGeometry *parent, uint depth) {
    IGeometry *geo;
    log_assert(depth < 8, "GL Render", "Renderer has 8 stencil buffers");

    // draw child geometries without mask
    gl_renderer_mask(&r, RENDER_DRAW_NO_MASK, depth);
    for (int geo_num = 0; geo_num < page->len_geometry; geo_num++) {
        geo = page->geometry[geo_num];
        if (geo == NULL) {
            continue;
        }

        if (geo->parent_id != parent->geo_id) {
            continue;
        }

        if (geo->mask_geo) {
            continue;
        }

        gl_render_draw_geometry(&r, geo);
    }

    gl_renderer_use(&r);
    gl_renderer_draw(&r);

    // draw child geometries with mask
    gl_renderer_mask(&r, RENDER_DRAW_MASK, depth);
    for (int geo_num = 0; geo_num < page->len_geometry; geo_num++) {
        geo = page->geometry[geo_num];
        if (geo == NULL) {
            continue;
        }

        if (geo->parent_id != parent->geo_id) {
            continue;
        }

        if (!geo->mask_geo) {
            continue;
        }

        gl_render_draw_geometry(&r, geo);
    }

    gl_renderer_use(&r);
    gl_renderer_draw(&r);

    for (int geo_num = 0; geo_num < page->len_geometry; geo_num++) {
        geo = page->geometry[geo_num];
        if (geo == NULL) {
            continue;
        }

        if (geo->parent_id != parent->geo_id) {
            continue;
        }

        if (!gl_render_has_child(page, geo_num)) {
            continue;
        }

        gl_render_clear_bit(page, depth);

        gl_renderer_mask(&r, RENDER_MASK, depth);
        gl_render_draw_geometry(&r, geo);

        gl_renderer_use(&r);
        gl_renderer_draw(&r);

        gl_render_draw_heirachy(page, geo, depth + 1);
    }
}

gboolean gl_render(GtkGLArea *area, GdkGLContext *context) {
    int page_index;
    float time, bezier_time;
    IPage *page;
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (int layer = 0; layer < CHROMA_LAYERS; layer++) {
        if (page_num[layer] < 0) {
            continue;
        }

        page_index = graphics_hub_get_page(&engine.hub, page_num[layer]);
        if (page_index < 0) {
            log_file(LogWarn, "GL Render", "Missing page %s", page_num[layer]);
            continue;
        }

        page = &engine.hub.items[page_index];

        switch (action[layer]) {
            case ANIMATE_OFF:
                if (current_page[layer] != page_num[layer]) {
                    action[layer] = BLANK;
                    break;
                }

            case ANIMATE_ON:
            case CONTINUE:
                current_page[layer] = page_num[layer];
                bezier_time = gl_bezier_time_step(frame_time[layer], 0, 1, 3);
                time = MIN(bezier_time + frame_num[layer], page->max_keyframe - 1);

                if (frame_time[layer] < 0 || bezier_time < 0) {
                    log_file(LogError, "GL Renderer", "Time less than 0: page %d keyframe %d", 
                             page_num[layer], frame_num[layer] - 1);
                }

                graphics_page_interpolate_geometry(page, time * ANIM_LENGTH, ANIM_LENGTH);
                frame_time[layer] = MIN(frame_time[layer] + 1.0f / ANIM_LENGTH, 1.0); 
                break;

            case BLANK:
            case UPDATE:
                break;

            default:
                log_file(LogError, "GL Render", "Unknown action %d", action);
        }

        if (action[layer] == BLANK) {
            continue;
        }

        if (action[layer] == UPDATE) {
            log_file(LogWarn, "GL Renderer", "Action update not handled before renderer");
            continue;
        }

        gl_render_draw_heirachy(page, page->geometry[0], 0);
        glClear(GL_STENCIL_BUFFER_BIT);
    }

    glFlush();
    
    return TRUE;
}
