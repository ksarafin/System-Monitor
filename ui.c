#include <gtk/gtk.h>
#include <math.h>

#include <gdk/gdkx.h>
#include "ui.h"
extern GtkWidget *main_window;

GtkWidget *createScrolledWindow(GtkWidget *treeView) {
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolledWindow), 200);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolledWindow), 400);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), treeView);

    return scrolledWindow;
}

gboolean draw_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    gint width, height;
    GdkWindow *window = gtk_widget_get_window(widget);

    width = gdk_window_get_width(window);
    height = gdk_window_get_height(window);

    /* Create a new surface for drawing */

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *temp_cr = cairo_create(surface);

    /* Background color */

    cairo_set_source_rgb(temp_cr, 1, 1, 1);
    cairo_paint(temp_cr);

    cairo_set_source_rgb(temp_cr, 0, 0, 0);

    cairo_set_line_width(temp_cr, 1);

    cairo_move_to(temp_cr, 50, 10);
    cairo_line_to(temp_cr, 50, height - 10);
    cairo_stroke(temp_cr);

    cairo_move_to(temp_cr, 50, height - 10);
    cairo_line_to(temp_cr, width - 10, height - 10);
    cairo_stroke(temp_cr);

    cairo_set_source_rgba(temp_cr, 0.5, 0.5, 0.5, 0.5);
    for (double y = 20; y < height - 10; y += (height - 30) / 10.0) {
        cairo_move_to(temp_cr, 50, y);
        cairo_line_to(temp_cr, width - 10, y);
    }
    cairo_stroke(temp_cr);

    GraphData *graph_data = (GraphData *)user_data;
    if (graph_data->history) {
        double interval_width = (double)(width - 60) / (MAX_HISTORY - 1);

        cairo_set_source_rgb(temp_cr, 0, 0, 1);
        cairo_set_line_width(temp_cr, 2);

        double x = 50;

        GList *iter;
        for (iter = graph_data->history; iter != NULL; iter = g_list_next(iter)) {
            double value = GPOINTER_TO_INT(iter->data);
            double y = height - (value * (height - 30) / 100.0 + 10);

            if (x == 50) {
                cairo_move_to(temp_cr, x, y);
            } else {
                cairo_line_to(temp_cr, x, y);
            }

            x += interval_width;
        }

        cairo_stroke(temp_cr);
    }

    cairo_set_source_rgb(temp_cr, 0, 0, 0);
    cairo_select_font_face(temp_cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(temp_cr, 12);

    cairo_move_to(temp_cr, 10, height / 2);
    cairo_rotate(temp_cr, -M_PI / 2.0);
    cairo_show_text(temp_cr, graph_data->label_y);
    cairo_rotate(temp_cr, M_PI / 2.0);

    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    cairo_surface_destroy(surface);
    cairo_destroy(temp_cr);

    return FALSE;
}
