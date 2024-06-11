/*
 * gl_render.c 
 *
 * Callbacks for GTK_GL_AREA used in preview.c and engine.c
 */

#include "geometry.h"
#include "gtk/gtk.h"
#include "gl_render_internal.h"

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "gl_math.h"
#include "log.h"
#include <time.h>

#define ANIM_LENGTH     60

int action[] = {BLANK, BLANK, BLANK, BLANK, BLANK};
int page_num[] = {-1, -1, -1, -1, -1};
int current_page[] = {-1, -1, -1, -1, -1};
int frame_num[] = {0, 0, 0, 0, 0};
float frame_time[] = {0.0, 0.0, 0.0, 0.0, 0.0};

/* read shader file */
char *gl_renderer_get_shader_file(char *filename) {
    FILE *file;
    char *shaderSource;

    file = fopen(filename, "rb");
    if (file == NULL) {
        shaderSource = "";
    } else {
        fseek(file, 0L, SEEK_END);
        long size = ftell(file) + 1;
        fclose(file);

        file = fopen(filename, "r");
        shaderSource = NEW_ARRAY(size, char);
        memset(shaderSource, '\0', size);
        fread(shaderSource, 1, size - 1, file);
        fclose(file);
    }

    return shaderSource; 
}

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

    gl_rectangle_init_buffers();
    gl_rectangle_init_shaders();

    gl_circle_init_buffers();
    gl_circle_init_shaders();

    gl_graph_init_buffers();
    gl_graph_init_shaders();

    gl_text_init_buffers();
    gl_text_init_shaders();
    gl_text_cache_characters();

    gl_image_init_buffers();
    gl_image_init_shaders();
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

    glUseProgram(0);
}

static float gl_bezier_time_step(float time, float start, float end, int order) {
    if (order == 1) {
        return (1 - time) * start + time * end;
    }

    float p1 = gl_bezier_time_step(time, start, end, order - 1);
    float p2 = gl_bezier_time_step(time, end, end, order - 1);

    return (1 - time) * p1 + time * p2;
}

gboolean gl_render(GtkGLArea *area, GdkGLContext *context) {
    int num_geo;
    float time, bezier_time;
    IPage *page;
    IGeometry *geo, *parent_geo;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(0);

    for (int layer = 0; layer < CHROMA_LAYERS; layer++) {
        if (page_num[layer] < 0) {
            continue;
        }

        page = graphics_hub_get_page(engine.hub, page_num[layer]);
        num_geo = graphics_page_num_geometry(page);

        switch (action[layer]) {
            case ANIMATE_OFF:
                if (current_page[layer] != page_num[layer]) {
                    action[layer] = BLANK;
                    break;
                }

            case ANIMATE_ON:
            case CONTINUE:
                current_page[layer] = page_num[layer];
                time = frame_time[layer] - frame_num[layer] + 1;
                bezier_time = gl_bezier_time_step(time, 0, 1, 3);

                if (time < 0 || bezier_time < 0) {
                    log_file(LogError, "GL Renderer", "Time less than 0: page %d keyframe %d", 
                             page_num[layer], frame_num[layer] - 1);
                }

                graphics_page_interpolate_geometry(
                    page, (bezier_time + frame_num[layer] - 1) * ANIM_LENGTH, ANIM_LENGTH);

                //log_file(LogMessage, "GL Renderer", "Time: %d Frame Time: %f Frame Num: %d", 
                //         time, frame_time[layer], frame_num[layer]); 

                frame_time[layer] = MIN(frame_time[layer] + 1.0f / ANIM_LENGTH, frame_num[layer]); 
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
        
        graphics_page_update_geometry(page);

        for (int geo_num = 0; geo_num < num_geo; geo_num++) {
            geo = graphics_page_get_geometry(page, geo_num);
            if (geo == NULL) {
                continue;
            }

            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_NEVER, 1, 0xFF);

            if (geo->mask_geo == 0) {
                glStencilMask(0x00);
            } else {
                glStencilMask(0xFF);
            }

            parent_geo = graphics_page_get_geometry(page, geo->parent);

            switch (parent_geo->geo_type) {
                case RECT:
                    gl_draw_rectangle(parent_geo);
                    break;
                case CIRCLE:
                    gl_draw_circle(parent_geo);
                    break;
                case TEXT:
                    gl_draw_text(parent_geo);
                    break;
                case GRAPH:
                    gl_draw_graph(parent_geo);
                    break;
                case IMAGE:
                    gl_draw_image(parent_geo);
                    break;
                default:
                    log_file(LogWarn, "GL Renderer", "Unknown geo type (%d)", parent_geo->geo_type);
            }

            if (geo->mask_geo == 0) {
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
            } else {
                glStencilFunc(GL_EQUAL, 1, 0xFF);
            }

            glStencilMask(0xFF);
            glEnable(GL_BLEND);

            switch (geo->geo_type) {
                case RECT:
                    gl_draw_rectangle(geo);
                    break;
                case CIRCLE:
                    gl_draw_circle(geo);
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
                default:
                    log_file(LogWarn, "GL Renderer", "Unknown geo type (%d)", geo->geo_type);
            }

            glDisable(GL_BLEND);
            glClear(GL_STENCIL_BUFFER_BIT);
        }
    }

    glFlush();
    
    return TRUE;
}
