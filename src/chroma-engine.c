/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "log.h"

#include <bits/getopt_core.h>
#include <stdio.h>
#include <sys/socket.h>

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *gl_area;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Chroma Engine");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    gl_area = chroma_new_renderer();
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    int x;
    int hflag = 0;
    int cflag = 0;

    char *config_path;
    opterr = 0;

    log_start(-1);

    while ((x = getopt(argc, argv, "c:hw:")) != -1) {
        switch (x) {
            case 'c':
                cflag = 1;
                config_path = optarg;
                break;

            case 'h':
                hflag = 1;
                break;

            default:
                log_file(LogError, "Engine", "Invalid Args %d", x);
        }
    }

    if (hflag) {
        printf("Usage:\n");
        printf("  -c [file]\tConfig File\n");
        return 0;
    }

    GtkApplication *app;
    int status;

    app = gtk_application_new("com.chroma.engine", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}
