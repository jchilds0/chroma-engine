/*
 * chroma-engine.c 
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "glib.h"
#include "log.h"

static gboolean chroma_key_press(GtkWidget *widget, GdkEventKey* event, gpointer data) {
    if (event->keyval != GDK_KEY_F10) {
        return FALSE;
    }

    g_mutex_lock(&engine.lock);
    engine.render_perf = !engine.render_perf;
    g_mutex_unlock(&engine.lock);

    return TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *gl_area;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Chroma Engine");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(chroma_key_press), NULL);

    gl_area = chroma_new_renderer();
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    int x;
    int hflag = 0;
    int cflag = 0;

    char *config_path = "";
    opterr = 0;

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
                printf("Invalid Args %d", x);
                return 1;
        }
    }

    if (hflag) {
        printf("Usage:\n");
        printf("  -c [file]\tConfig File\n");
        return 0;
    }

    if (chroma_init_renderer(config_path, "./log") < 0) {
        log_file(LogWarn, "Engine", "Error initialising renderer");
        return 1;
    }

    GtkApplication *app;
    int status;

    // use deprecated flags none while ubuntu packages are below glib 2.74
    app = gtk_application_new("com.chroma.engine", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}
