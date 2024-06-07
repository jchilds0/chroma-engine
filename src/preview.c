/*
 * Creates a preview process which embeds in another GTK application using GtkPlug.
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "chroma-prototypes.h"
#include "gl_render.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "parser.h"
#include "log.h"
#include <X11/Xlib.h>
#include <sys/socket.h>

static void close_preview(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Preview", "Shutdown");
    gtk_main_quit();

    shutdown(engine.server_socket, SHUT_RDWR);
    graphics_free_graphics_hub(engine.hub);
    exit(1);
}

void preview_window(int wid) {
    GtkWidget *plug, *gl_area;
    int p_page_num = -1, p_action = 0, layer = 0;
    gtk_init(0, NULL);

    engine.server_port = 6100;
    engine.server_socket = parser_tcp_start_server("127.0.0.1", engine.server_port);

    plug = gtk_plug_new(wid);
    gl_area = gtk_gl_area_new();

    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), gtk_true());

    g_signal_connect(G_OBJECT(plug), "destroy", G_CALLBACK(close_preview), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    gtk_container_add(GTK_CONTAINER(plug), gl_area);
    gtk_widget_show_all(plug);

    while (TRUE) {
        gtk_main_iteration_do(FALSE);
        parser_check_socket(engine.server_socket);
        parser_parse_graphic(&engine, &p_page_num, &p_action, &layer);

        page_num[0] = p_page_num;
        action[0] = p_action;
    }
}

