/*
 * Callbacks for GTK_GL_AREA used in preview.c and engine.c
 */

#include "chroma-engine.h"
#include "chroma-prototypes.h"
#include "chroma-typedefs.h"
#include "gl_renderer.h"
#include <GL/gl.h>

Action action = BLANK;
int page_num = -1;

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

    // setup circle 
    gl_circle_init_buffers();
    gl_circle_init_shaders();
    
    // setup annulus
    gl_annulus_init_buffers();
    gl_annulus_init_shaders();
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
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(0);
    
    switch (action) {
        case BLANK:
            break;
        case ANIMATE_ON:
            engine.hub->pages[page_num]->page_animate_on(page_num, engine.hub->time);
            engine.hub->time = MIN(engine.hub->time + 1.0 / CHROMA_FRAMERATE, 1.1); 
            engine.hub->current_page = page_num;

            break;
        case CONTINUE:
            engine.hub->pages[page_num]->page_continue(page_num, engine.hub->time);
            engine.hub->time = MIN(engine.hub->time + 1.0 / CHROMA_FRAMERATE, 1.1); 

            break;
        case ANIMATE_OFF:
            if (engine.hub->current_page != page_num) {
                break;
            }

            engine.hub->pages[page_num]->page_animate_off(page_num, 1.0 - engine.hub->time);
            engine.hub->time = MIN(engine.hub->time + 1.0 / CHROMA_FRAMERATE, 1.1); 
            break;
        default:
            log_file(LogError, "Unknown action %d", action);
    }

    if (!WITHIN(page_num, 0, engine.hub->num_pages)) {
        //log_file(LogWarn, "page num out of range %d", page_num);
        return TRUE;
    }

    Page *page = engine.hub->pages[page_num];
    for (int i = 0; i < page->num_rect; i++) {
        gl_rect_render(&page->rect[i]);
    }

    for (int i = 0; i < page->num_circle; i++) {
        gl_circle_render(&page->circle[i]);
    }

    for (int i = 0; i < page->num_annulus; i++) {
        gl_annulus_render(&page->annulus[i]);
    }

    for (int i = 0; i < page->num_text; i++) {
        gl_text_render(&page->text[i], 1.0);
    }

    gl_rect_render(&page->mask);

    glFlush();
    
    return TRUE;
}

// ChromaCircle *circle = NEW_STRUCT(ChromaCircle);
// circle->center_x = 500;
// circle->center_y = 500;
// circle->radius = 100;
// circle->color[0] = 1.0;
// circle->color[1] = 0.0;
// circle->color[2] = 1.0;
// circle->color[3] = 1.0;
// gl_circle_render(circle);

// Chroma_Rectangle *rect = NEW_STRUCT(Chroma_Rectangle);
// *rect = (Chroma_Rectangle) {100, 100, 100, 100};
// rect->color[0] = 1.0;
// rect->color[1] = 0.0;
// rect->color[2] = 0.0;
// rect->color[3] = 1.0;
// gl_rect_render(rect);

// Chroma_Text *text = NEW_STRUCT(Chroma_Text);
// text->pos_x = 200;
// text->pos_y = 200;
// memset(text->buf, '\0', sizeof text->buf);
// memcpy(text->buf, "This is sample text\0", 21);
// text->color[0] = 1.0;
// text->color[1] = 1.0;
// text->color[2] = 1.0;
// text->color[3] = 1.0;
//
// GLfloat mat[] = GL_MATH_ROTATE_X(DEG_TO_RAD(1.0f));
//
// for (int i = 0; i < 16; i++) {
//     text->transform[i] = mat[i];
// }
//
// gl_text_render(text, 1.0f);
