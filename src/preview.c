/*
 * Creates a preview process which embeds in another GTK application using GtkPlug.
 */

#include "chroma-engine.h"

gboolean draw_preview(GtkWidget *, cairo_t *, gpointer);

gboolean draw_preview(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width, height;
    width = gtk_widget_get_allocated_width(widget);
    height = width * 9 / 16;

    // set bg to black
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    //renderer(cr, (Engine *)data);

    return FALSE;
}

void close_plug(GtkWidget *, gpointer);

void close_plug(GtkWidget *widget, gpointer data) {
    log_to_file(LOG_MESSAGE, "Closed plug");
    gtk_main_quit();
}

void preview_window(int wid, Engine *preview) {
    gtk_init(0, NULL);

    GtkWidget *plug = gtk_plug_new(wid);
    GtkWidget *da = gtk_drawing_area_new();

    g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(draw_preview), preview);
    g_signal_connect(G_OBJECT(plug), "destroy", G_CALLBACK(close_plug), NULL);

    gtk_container_add(GTK_CONTAINER(plug), da);
    gtk_widget_show_all(plug);

    gtk_main();
}

