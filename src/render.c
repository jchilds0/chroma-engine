/*
 * render.c 
 */

#include "chroma-typedefs.h"
#include "config.h"
#include "parser.h"
#include "gl_render.h"
#include "log.h"

#include "gtk/gtk.h"
#include <string.h>
#include <sys/socket.h>

Engine engine = {.active = 0};
Config config = {
    .hub_addr = "127.0.0.1",
    .hub_port = 9000,
    .engine_port = 6100,
};

static PageStatus status = {-1, 0, 0, BLANK};

static void chroma_close_renderer(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Engine", "Shutdown");

    shutdown(engine.server_socket, SHUT_RDWR);
    graphics_free_graphics_hub(&engine.hub);
    engine.active = 0;
}

int chroma_init_renderer(char *config_path, char *log_path) {
    parser_init_sockets();

    log_start(log_path);

    if (strlen(config_path) > 0) {
        config_parse_file(&config, config_path);
    }

    sprintf(engine.hub_addr, "%s:%d", config.hub_addr, config.hub_port); 
    engine.hub_socket = parser_tcp_start_client(config.hub_addr, config.hub_port);
    if (engine.hub_socket < 0) {
        return -1;
    }

    log_file(LogMessage, "Engine", "Graphics hub %s:%d", config.hub_addr, config.hub_port); 

    clock_t start, end;
    start = clock();

    if (parser_parse_hub(&engine) < 0) {
        return -1;
    };

    end = clock();

    engine.server_port = config.engine_port;
    engine.server_socket = parser_tcp_start_server(engine.server_port);
    if (engine.server_socket < 0) {
        return -1;
    }

    engine.active = 1;

    log_file(LogMessage, "Parser", "Imported Chroma Hub in %f ms", ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
    return 0;
}

static int chroma_renderer_draw(gpointer data) {
    if (!engine.active) {
        log_file(LogMessage, "Engine", "Engine is not active, closing main loop");
        return 0;
    }

    parser_check_socket(engine.server_socket);
    if (parser_parse_graphic(&engine, &status) < 0) {
        log_file(LogMessage, "Engine", "Error receiving graphic, closing main loop");
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
    
    gl_area = gtk_gl_area_new();
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), gtk_true());
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), gtk_false());

    g_signal_connect(G_OBJECT(gl_area), "destroy", G_CALLBACK(chroma_close_renderer), NULL);
    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    g_idle_add(chroma_renderer_draw, NULL);
    return gl_area;
}
