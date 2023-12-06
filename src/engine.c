/*
 * Creates a seperate window using GTK to run the engine process.
 */

#include "chroma-engine.h"
#include "chroma-prototypes.h"
#include <GL/gl.h>

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2, 1.0f);\n"
    "}\0";

static float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
};

static GLuint position_buffer;
static GLuint program;

/* Initialize the GL buffers */
static void init_buffers (GLuint *vao_out, GLuint *buffer_out) {
    GLuint vao, buffer;

    /* We only use one VAO, so we always keep it bound */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* This is the buffer that holds the vertices */
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (vao_out != NULL)
        *vao_out = vao;

    if (buffer_out != NULL)
        *buffer_out = buffer;
}

/* Create and compile a shader */
static GLuint create_shader (int type, const char *src) {
    GLuint shader;
    int status;

    shader = glCreateShader (type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
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

static void realize(GtkWidget *widget) {
    GLuint vertex, fragment;
    int status;

    gtk_gl_area_make_current(GTK_GL_AREA(widget));
    glewInit();

    /* create buffer */
    init_buffers(NULL, &position_buffer);

    /* create shader */
    vertex = create_shader(GL_VERTEX_SHADER, vertexShaderSource);
    fragment = create_shader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

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

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

static gboolean render(GtkGLArea *area, GdkGLContext *context) {
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    /* triangle */ 

    // Copy vertices array in a buffer for OpenGL
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    // Set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *) 0);
    glEnableVertexAttribArray(0);

    // Use the shader program when we want to render an object
    glUseProgram(program);

    // draw triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // finished with buffers and program
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    glFlush();
    return TRUE;
}

void engine_window(void) {
    GtkWidget *window, *gl_area;

    gtk_init(0, NULL);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gl_area = gtk_gl_area_new();

    //engine.port = 6800;
    //engine.socket = start_tcp_server("127.0.0.1", engine.port);

    gtk_window_set_title(GTK_WINDOW(window), "Chroma Engine");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(free_engine), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(realize), NULL);

    gtk_container_add(GTK_CONTAINER(window), gl_area);
    gtk_widget_show_all(window);

    while (TRUE) {
        gtk_main_iteration();
    }
}

