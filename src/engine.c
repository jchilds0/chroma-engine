/*
 * Creates a seperate window using GTK to run the engine process.
 */

#include "cairo.h"
#include "chroma-engine.h"

gboolean draw_engine(GtkWidget *, cairo_t *, gpointer);

gboolean draw_engine(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width, height;
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    // set bg to black
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    //renderer(cr, (Engine *)data);

    return FALSE;
}

void engine_window(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    GtkWidget *da = gtk_drawing_area_new();
    gtk_window_set_title(GTK_WINDOW(window), "Chroma Engine");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

    g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(draw_engine), user_data);
    gtk_container_add(GTK_CONTAINER(window), da);

    gtk_widget_show_all(window);
}

