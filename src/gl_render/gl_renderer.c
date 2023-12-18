/*
 * Callbacks for GTK_GL_AREA used in preview.c and engine.c
 */

#include "gl_render_internal.h"

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "gl_math.h"
#include "graphics.h"
#include "log.h"

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

gboolean gl_render(GtkGLArea *area, GdkGLContext *context) {
    int current_page;
    float time;
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(0);
    
    switch (action) {
        case BLANK:
            break;
        case ANIMATE_ON:
            graphics_page_update_on(engine.hub, page_num);
            time = graphics_hub_get_time(engine.hub);
            time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.1); 
            graphics_hub_set_time(engine.hub, time);
            graphics_hub_set_current_page_num(engine.hub, page_num);

            break;
        case CONTINUE:
            graphics_page_update_cont(engine.hub, page_num);
            time = graphics_hub_get_time(engine.hub);
            time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.1); 
            graphics_hub_set_time(engine.hub, time);

            break;
        case ANIMATE_OFF:
            current_page = graphics_hub_get_current_page_num(engine.hub);
            if (current_page != page_num) {
                break;
            }

            time = graphics_hub_get_time(engine.hub);
            graphics_hub_set_time(engine.hub, 1.0 - time);

            graphics_page_update_off(engine.hub, page_num);

            time = MIN(time + 1.0 / CHROMA_FRAMERATE, 1.1); 
            graphics_hub_set_time(engine.hub, time);
            break;
        default:
            log_file(LogError, "GL Render", "Unknown action %d", action);
    }

    IPage *page = graphics_hub_get_page(engine.hub, page_num);
    IGeometry *geo;
    int num_geo = graphics_page_num_geometry(page);
    char geo_type[20];

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

        } else {

            log_file(LogWarn, "GL Renderer", "Unknown geo type (%s)", geo_type);

        }
    }

    glFlush();
    
    return TRUE;
}
