/*
 * Callbacks for GTK_GL_AREA used in preview.c and engine.c
 */

#include "gl_renderer.h"
#include "chroma-engine.h"
#include "chroma-prototypes.h"
#include "chroma-typedefs.h"
#include <math.h>

#define DEG_TO_RAD(theta)     theta * M_PI / 180

#define ROTATE(theta)         {                                \
                                cosf(theta),-sinf(theta), 0.0, \
                                sinf(theta), cosf(theta), 0.0, \
                                0.0        , 0.0        , 1.0, \
                              }

Action action;
int page_num;

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

    // setup text 
    gl_text_init_buffers();
    gl_text_init_shaders();
    gl_text_cache_characters();

    // setup rect
    gl_rect_init_buffers();
    gl_rect_init_shaders();
}

void gl_renderer_set_scale(GLuint program) {
    GLfloat matrix[] = { 
        2.0f / 1920, 0.0f,
        0.0f, 2.0f / 1080,
    };

    glUseProgram(program);

    uint size_loc = glGetUniformLocation(program, "scale");
    glUniformMatrix2fv(size_loc, 1, GL_FALSE, matrix);

    glUseProgram(0);
}

gboolean gl_render(GtkGLArea *area, GdkGLContext *context) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(0);

    // Chroma_Rectangle *rect = NEW_STRUCT(Chroma_Rectangle);
    // *rect = (Chroma_Rectangle) {100, 100, 100, 100};
    // set_color(&rect->color[0], 255, 0, 255, 255);
    // gl_rect_render(rect);

    // Chroma_Text *text = NEW_STRUCT(Chroma_Text);
    // text->pos_x = 100;
    // text->pos_y = 100;
    // memset(text->buf, '\0', sizeof text->buf);
    // memcpy(text->buf, "This is sample text\0", 21);
    // set_color(&text->color[0], 255, 0, 0, 255);
    // gl_text_render(text, 1.0);

    switch (action) {
        case BLANK:
            break;
        case ANIMATE_ON:
            page_animate_off(engine.hub->current_page);
            page_animate_on(page_num);

            break;
        case CONTINUE:
            page_continue(page_num);

            break;
        case ANIMATE_OFF:
            page_animate_off(page_num);

            break;
        default:
            log_file(LogError, "Unknown action %d", action);
    }

    glFlush();
    
    return TRUE;
}

