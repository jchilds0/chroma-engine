/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <malloc.h>
#include <unistd.h>

int main(int argc, char **argv) {
    GtkApplication *app;
    Engine engine;
    uint wid;
    start_log();

    // Graphics Hub 
    engine.hub = init_hub(10);
    Page *page = init_page(1);
    set_rect(page, 0, 50, 50, 300, 50);
    add_graphic(engine.hub, page);

    page = init_page(1);
    set_rect(page, 0, 50, 780, 500, 200);
    add_graphic(engine.hub, page);

    page = init_page(1);
    set_rect(page, 0, 100, 100, 300, 200);
    add_graphic(engine.hub, page);

    switch (argc) {
    case 2:
        if (strcmp(argv[1], "--engine") == 0) {
            engine.port = 6800;
            engine.socket = start_tcp_server("127.0.0.1", engine.port);

            app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
            g_signal_connect(app, "activate", G_CALLBACK(engine_window), &engine);
            g_application_run(G_APPLICATION(app), 0, NULL);
            g_object_unref(app);
            break;
        }

        log_to_file(LOG_ERROR, "incorrect argument %s", argv[1]);
        return 0;
    case 3:
        if (strcmp(argv[1], "-wid") == 0) {
            engine.port = 6800;
            engine.socket = start_tcp_server("127.0.0.1", engine.port);
            wid = atoi(argv[2]);
            log_to_file(LOG_MESSAGE, "Recieved wid %d", wid);
            preview_window(wid, &engine);
            break;
        }

        log_to_file(LOG_ERROR, "Incorrect argument %s", argv[1]);
        return 0;
    default: 
        log_to_file(LOG_ERROR, "Incorrect usage");
        return 0;
    }

    shutdown(engine.socket, SHUT_RDWR);
    free_hub(engine.hub);
    log_to_file(LOG_MESSAGE, "Engine Shutdown");

    return 1;
}

