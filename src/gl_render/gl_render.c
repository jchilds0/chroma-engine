/*
 * Callbacks for GTK_GL_AREA used in preview.c and engine.c
 */

#include "gl_render_internal.h"

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "gl_math.h"
#include "graphics.h"
#include "log.h"
#include <GL/gl.h>
#include <string.h>

int action = BLANK;
int page_num = 0;

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

    gl_rectangle_init_buffers();
    gl_rectangle_init_shaders();

    gl_circle_init_buffers();
    gl_circle_init_shaders();

    gl_annulus_init_buffers();
    gl_annulus_init_shaders();

    gl_graph_init_buffers();
    gl_graph_init_shaders();

    gl_text_init_buffers();
    gl_text_init_shaders();
    gl_text_cache_characters();
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

static float gl_bezier_time_step(float time, float start, float end) {
    float p0[] = {0, start};
    float p1[] = {0, end};
    float p2[] = {1, end};

    float b_x = (1 - time) * (1 - time) * p0[0] 
        + 2 * time * (1 - time) * p1[0] 
        + time * time * p2[0];

    float b_y = (1 - time) * (1 - time) * p0[1] 
        + 2 * time * (1 - time) * p1[1]
        + time * time * p2[1];

    return b_y;
}

gboolean gl_render(GtkGLArea *area, GdkGLContext *context) {
    int current_page, num_geo;
    char geo_type[20];
    float time, bezier_time;
    IPage *page;
    IGeometry *geo;

    page = graphics_hub_get_page(engine.hub, page_num);
    num_geo = graphics_page_num_geometry(page);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(0);
    
    switch (action) {
        case BLANK:
            break;
        case ANIMATE_ON:
            time = graphics_hub_get_time(engine.hub);
            bezier_time = gl_bezier_time_step(time, 0.0, 1.1);
            graphics_page_update_animation(page, "animate_on", bezier_time);

            time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.0); 
            graphics_hub_set_time(engine.hub, time);
            graphics_hub_set_current_page_num(engine.hub, bezier_time);

            break;
        case CONTINUE:
            time = graphics_hub_get_time(engine.hub);
            bezier_time = gl_bezier_time_step(time, 0.0, 1.1);
            graphics_page_update_animation(page, "continue", bezier_time);

            time = graphics_hub_get_time(engine.hub);
            time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.0); 
            graphics_hub_set_time(engine.hub, time);

            break;
        case ANIMATE_OFF:
            current_page = graphics_hub_get_current_page_num(engine.hub);
            if (current_page != page_num) {
                break;
            }

            time = graphics_hub_get_time(engine.hub);
            bezier_time = gl_bezier_time_step(time, 1.1, 0.0);
            graphics_page_update_animation(page, "animate_off", bezier_time);

            time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.0); 
            graphics_hub_set_time(engine.hub, time);
            break;
        default:
            log_file(LogError, "GL Render", "Unknown action %d", action);
    }

    for (int geo_num = 0; geo_num < num_geo; geo_num++) {
        memset(geo_type, '\0', sizeof geo_type);
        geo = graphics_page_get_geometry(page, geo_num);
        geometry_get_attr(geo, "geo_type", geo_type);
        
        if (strncmp(geo_type, "rect", 4) == 0) {

            gl_draw_rectangle(geo);

        } else if (strncmp(geo_type, "circle", 5) == 0) {

            gl_draw_circle(geo);

        } else if (strncmp(geo_type, "annulus", 7) == 0) {

            gl_draw_annulus(geo);

        } else if (strncmp(geo_type, "text", 4) == 0) {

            gl_draw_text(geo);

        } else if (strncmp(geo_type, "graph", 5) == 0) {

            gl_draw_graph(geo);

        } else {

            log_file(LogWarn, "GL Renderer", "Unknown geo type (%s)", geo_type);

        }
    }

    glFlush();
    
    return TRUE;
}
