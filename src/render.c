/*
 *
 */

#include "chroma-typedefs.h"
#include "config.h"
#include "graphics.h"
#include "parser.h"
#include "gl_render.h"
#include "log.h"

#include "glib.h"
#include "gtk/gtk.h"
#include <sys/socket.h>

Engine engine;
Config config = {
    .hub_addr = "127.0.0.1",
    .hub_port = 9000
};

static PageStatus status = {-1, 0, 0, BLANK};

static void chroma_close_renderer(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Engine", "Shutdown");

    shutdown(engine.server_socket, SHUT_RDWR);
    graphics_free_graphics_hub(&engine.hub);
}

static int chroma_init_renderer(void) {
    parser_init_sockets();

    sprintf(engine.hub_addr, "%s:%d", config.hub_addr, config.hub_port); 
    engine.hub_socket = parser_tcp_start_client(config.hub_addr, config.hub_port);
    log_file(LogMessage, "Engine", "Graphics hub %s:%d", config.hub_addr, config.hub_port); 

    clock_t start, end;
    start = clock();

    if (parser_parse_hub(&engine) < 0) {
        return -1;
    };

    end = clock();

    log_file(LogMessage, "Parser", "Imported Chroma Hub in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
    return 0;
}

static int chroma_renderer_draw(gpointer data) {
    parser_check_socket(engine.server_socket);
    if (parser_parse_graphic(&engine, &status) < 0) {
        return 0;
    }

    if (status.action == BLANK) {
        return 1;
    }

    page_num[status.layer]  = status.temp_id;
    action[status.layer]    = status.action;
    frame_num[status.layer] = status.frame_num;
    frame_time[status.layer] = 0.0;

    return 1;
}

GtkWidget *chroma_new_renderer(void) {
    GtkWidget *gl_area;
    
    log_start(LogEngine);
    gl_area = gtk_gl_area_new();
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), gtk_true());
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), gtk_false());

    if (chroma_init_renderer() < 0) {
        return gl_area;
    }

    engine.server_port = 6800;
    engine.server_socket = parser_tcp_start_server(engine.server_port);

    g_signal_connect(G_OBJECT(gl_area), "destroy", G_CALLBACK(chroma_close_renderer), NULL);

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    g_idle_add(chroma_renderer_draw, NULL);
    return gl_area;
}
