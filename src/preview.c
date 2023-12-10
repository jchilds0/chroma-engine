/*
 * Creates a preview process which embeds in another GTK application using GtkPlug.
 */

#include "chroma-engine.h"

static void close_plug(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Closed Plug");
    free_engine();
    gtk_main_quit();
}

void preview_window(int wid) {
    GtkWidget *plug, *gl_area;
    gtk_init(0, NULL);

    engine.port = 6100;
    engine.socket = parser_tcp_start_server("127.0.0.1", engine.port);

    plug = gtk_plug_new(wid);
    gl_area = gtk_gl_area_new();

    g_signal_connect(G_OBJECT(plug), "destroy", G_CALLBACK(close_plug), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    gtk_container_add(GTK_CONTAINER(plug), gl_area);
    gtk_widget_show_all(plug);

    while (TRUE) {
        gtk_main_iteration_do(FALSE);
        parser_read_socket(&page_num, &action);
    }
}

