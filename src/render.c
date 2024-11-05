/*
 * render.c 
 */

#include "chroma-macros.h"
#include "chroma-typedefs.h"
#include "config.h"
#include "parser.h"
#include "gl_render.h"
#include "log.h"

#include <pthread.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Engine engine;
Config config = {
    .hub_addr = "127.0.0.1",
    .hub_port = 9000,
    .engine_port = 6100,
};

pthread_mutex_t lock;
unsigned char active = 0;

static void chroma_close_renderer(GtkWidget *widget, gpointer data) {
    log_file(LogMessage, "Engine", "Shutdown");

    graphics_free_graphics_hub(&engine.hub);
    pthread_mutex_lock(&lock);
    active = 0;
    pthread_mutex_unlock(&lock);
}

static void *chroma_handle_conn(void *data) {
    PageStatus status;
    Client client = {
        .client_sock = *(int *)(data),
        .buf_ptr = 0,
    };
    int exit = 0;

    memset(&client.buf, '\0', sizeof client.buf);

    while (!exit) {
        pthread_mutex_lock(&lock);
        if (!active) {
            log_file(LogMessage, "Engine", "Engine is not active, closing client %d handler", client.client_sock);
            pthread_mutex_unlock(&lock);
            exit = 1;
            continue;
        }
        pthread_mutex_unlock(&lock);

        status = (PageStatus){.temp_id = 0, .frame_num = 0, .layer = 0, .action = BLANK};
        if (parser_parse_graphic(&engine, &client, &status) < 0) {
            exit = 1;
        }

        if (status.action == BLANK) {
            continue;
        }

        log_file(LogMessage, "Engine", "Recieved Graph: Temp ID %d, Layer %d, Frame Num %d", 
                 status.temp_id, status.layer, status.frame_num);

        pthread_mutex_lock(&gl_lock);
        page_num[status.layer]  = status.temp_id;
        action[status.layer]    = status.action;
        frame_num[status.layer] = status.frame_num;
        frame_time[status.layer] = 0.0;
        pthread_mutex_unlock(&gl_lock);
    }

    shutdown(client.client_sock, SHUT_RDWR);
    return NULL;
}

static void *chroma_listen(void *data) {
    unsigned char exit = 0;
    log_file(LogMessage, "Engine", "Starting main engine server");

    int server_sock = parser_tcp_start_server(engine.server_port);
    if (server_sock < 0) {
        log_file(LogMessage, "Engine", "Error creating socket, closing server");

        pthread_mutex_lock(&lock);
        active = 0;
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    while (!exit) {
        pthread_mutex_lock(&lock);
        if (!active) {
            log_file(LogMessage, "Engine", "Engine is not active, closing server");
            pthread_mutex_unlock(&lock);
            exit = 1;
            continue;
        }

        pthread_mutex_unlock(&lock);

        int client_sock = parser_accept_conn(server_sock);
        if (client_sock < 0) {
            log_file(LogMessage, "Engine", "Error connecting to new client");
            continue;
        }

        pthread_t th_client;
        pthread_create(&th_client, NULL, chroma_handle_conn, &client_sock);
    }

    shutdown(server_sock, SHUT_RDWR);
    return NULL;
}

int chroma_init_renderer(char *config_path, char *log_path) {
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
    pthread_mutex_lock(&lock);
    active = 1;
    pthread_mutex_unlock(&lock);

    pthread_t th_listen;
    pthread_create(&th_listen, NULL, chroma_listen, NULL);

    log_file(LogMessage, "Parser", "Imported Chroma Hub in %f ms", 
             ((double) (end - start) * 1000) / CLOCKS_PER_SEC);
    return 0;
}

GtkWidget *chroma_new_renderer(void) {
    GtkWidget *gl_area;
    
    gl_area = gtk_gl_area_new();
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), gtk_true());
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), gtk_false());

    g_signal_connect(G_OBJECT(gl_area), "destroy", G_CALLBACK(chroma_close_renderer), NULL);
    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(gl_render), NULL);
    g_signal_connect(G_OBJECT(gl_area), "realize", G_CALLBACK(gl_realize), NULL);

    return gl_area;
}
