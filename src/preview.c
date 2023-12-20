/*
 * Creates a preview process which embeds in another GTK application using GtkPlug.
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "gl_render.h"
#include "parser.h"
#include "log.h"
#include <sys/socket.h>

static void close_preview(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Preview", "Shutdown");
    gtk_main_quit();

    shutdown(engine.socket, SHUT_RDWR);
    graphics_free_graphics_hub(engine.hub);
    exit(1);
}

void preview_window(int wid) {
    GtkWidget *plug, *gl_area;
    gtk_init(0, NULL);

    engine.port = 6100;
    engine.socket = parser_tcp_start_server("127.0.0.1", engine.port);
    engine.hub = graphics_new_graphics_hub();
    graphics_hub_load_example(engine.hub);

    plug = gtk_plug_new(wid);
    gl_area = gtk_gl_area_new();

    g_signal_connect(G_OBJECT(plug), "destroy", G_CALLBACK(close_preview), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    gtk_container_add(GTK_CONTAINER(plug), gl_area);
    gtk_widget_show_all(plug);

    while (TRUE) {
        gtk_main_iteration_do(FALSE);
        parser_read_socket(&engine, &page_num, &action);
    }
}

