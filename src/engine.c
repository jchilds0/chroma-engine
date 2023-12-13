/*
 * Creates a seperate window using GTK to run the engine process.
 */

#include "chroma-engine.h"
#include <unistd.h>

void engine_window(void) {
    GtkWidget *window, *gl_area;
    gtk_init(0, NULL);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gl_area = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    engine.port = 6800;
    engine.socket = parser_tcp_start_server("127.0.0.1", engine.port);

    gtk_window_set_title(GTK_WINDOW(window), "Chroma Engine");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(free_engine), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    gtk_widget_show_all(window);

    while (TRUE) {
        // slow frame rate down to CHROMA_FRAMERATE
        gtk_main_iteration_do(FALSE);
        parser_read_socket(&page_num, &action);
    }
}

