#ifndef SYSTEM_UI_H
#define SYSTEM_UI_H

#include <gtk/gtk.h>


#define MAX_HISTORY 200

typedef struct {
    GList *history;
    char label_x[50];
    char label_y[50];
} GraphData;

GtkWidget *createScrolledWindow(GtkWidget *treeView);

gboolean draw_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data);

#endif
