/*
 * Creates a preview process which embeds in another GTK application using GtkPlug.
 */

#include "chroma-engine.h"

gboolean draw_preview(GtkGLArea *, GdkGLContext *);

gboolean draw_preview(GtkGLArea *area, GdkGLContext *context) {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1, 0, 0);
    glRectf(-0.5, -0.5, 0.5, 0.5);
    glFlush();

    return TRUE;
}

void close_plug(GtkWidget *, gpointer);

void close_plug(GtkWidget *widget, gpointer data) {
    log_to_file(LogMessage, "Closed plug");
    gtk_main_quit();
}

void preview_window(int wid) {
    GtkWidget *plug, *gl_area;

    engine.port = 6100;
    engine.socket = start_tcp_server("127.0.0.1", engine.port);

    gtk_init(0, NULL);

    plug = gtk_plug_new(wid);
    gl_area = gtk_gl_area_new();

    g_signal_connect(G_OBJECT(gl_area), "render", G_CALLBACK(draw_preview), NULL);
    g_signal_connect(G_OBJECT(plug), "destroy", G_CALLBACK(close_plug), NULL);

    gtk_container_add(GTK_CONTAINER(plug), gl_area);
    gtk_widget_show_all(plug);

    gtk_main();
}

