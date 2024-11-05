/*
 * Main header for Chroma Engine 
 *
 * All .c files should include this header.
 */

#ifndef CHROMA_ENGINE
#define CHROMA_ENGINE

#include <gtk/gtk.h>

int        chroma_init_renderer(char *config_path, char *log_path);
GtkWidget  *chroma_new_renderer(void);

#endif // !CHROMA_ENGINE
