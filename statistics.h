#ifndef STATISTICS_H
#define STATISTICS_H

#include <gtk/gtk.h>

extern int g_num_samples;
extern int g_delay_seconds;
extern long int g_last_total_time;
extern long int g_last_idle_time;

extern GtkWidget *cpu_label;
extern GtkWidget *memory_label;
extern GtkWidget *network_label;
extern GtkWidget *swap_label;
extern GtkWidget *cpu_graph;
extern GtkWidget *memory_graph;
extern GtkWidget *swap_graph;
extern GtkWidget *network_graph;

void calculate_memory_swap_usage(double *memory_usage, double *swap_usage);
gboolean draw_memory_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_swap_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data);
double calculate_network_usage();
gboolean draw_network_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data);
double calculate_cpu_usage();
gboolean draw_cpu_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void update_stats();
void create_graph(GtkWidget *grid, GtkWidget **graph, const gchar *label_text, GCallback draw_func);

#endif  // STATISTICS_H
