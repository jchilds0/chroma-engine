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

    shutdown(engine.server_socket, SHUT_RDWR);
    graphics_free_graphics_hub(engine.hub);
    exit(1);
}

void preview_window(int wid) {
    GtkWidget *plug, *gl_area;
    PageStatus status = {-1, 1, 0, BLANK};
    gtk_init(0, NULL);

    engine.server_port = 6100;
    engine.server_socket = parser_tcp_start_server(engine.server_port);

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
        if (parser_parse_graphic(&engine, &status) < 0) {
            continue;
        }

        page_num[status.layer]  = status.temp_id;
        action[status.layer]    = status.action;
        frame_num[status.layer] = status.frame_num;
        frame_time[status.layer] = 0.0;
    }
}

