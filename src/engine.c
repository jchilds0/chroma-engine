/*
 * Creates a seperate window using GTK to run the engine process.
 */

#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "gl_render.h"
#include "gtk/gtk.h"
#include "log.h"
#include "parser.h"
#include <sys/socket.h>
#include <unistd.h>

static void close_engine(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Engine", "Shutdown");

    shutdown(engine.server_socket, SHUT_RDWR);
    exit(1);
}

void engine_window(void) {
    GtkWidget *window, *gl_area;
    PageStatus status = {-1, 0, 0, BLANK};
    gtk_init(0, NULL);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gl_area = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), gtk_true());
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), gtk_false());

    engine.server_port = 6800;
    engine.server_socket = parser_tcp_start_server(engine.server_port);

    gtk_window_set_title(GTK_WINDOW(window), "Chroma Engine");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_engine), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    gtk_widget_show_all(window);

    while (TRUE) {
        // slow frame rate down to CHROMA_FRAMERATE
        gtk_main_iteration_do(FALSE);
        parser_check_socket(engine.server_socket);
        if (parser_parse_graphic(&engine, &status) < 0) {
            continue;
        }

        page_num[status.layer]  = status.temp_id;
        action[status.layer]    = status.action;
        frame_num[status.layer] = status.frame_num;
        frame_time[status.layer] = 0.0;
    }
}

