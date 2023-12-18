/*
 * Creates a seperate window using GTK to run the engine process.
 */

#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "gl_renderer.h"
#include "log.h"
#include "parser.h"
#include <sys/socket.h>
#include <unistd.h>

static void close_engine(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Preview", "Shutdown");

    shutdown(engine.socket, SHUT_RDWR);
    graphics_free_graphics_hub(engine.hub);
    exit(1);
}

void engine_window(void) {
    GtkWidget *window, *gl_area;
    gtk_init(0, NULL);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gl_area = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    engine.port = 6800;
    engine.socket = parser_tcp_start_server("127.0.0.1", engine.port);
    engine.hub = graphics_new_graphics_hub();

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
        parser_read_socket(&engine, &page_num, &action);
    }
}

