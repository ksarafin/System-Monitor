#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <gtk/gtk.h>

extern int g_num_samples;
extern int g_delay_seconds;
extern long int g_last_total_time;
extern long int g_last_idle_time;

extern GtkWidget *cpu_label;
extern GtkWidget *cpu_progress;
extern GtkWidget *memory_label;
extern GtkWidget *memory_progress;
extern GtkWidget *swap_label;
extern GtkWidget *swap_progress;

void calculate_memory_swap_usage(double *memory_usage, double *swap_usage);
void calculate_network_usage();
double calculate_cpu_usage();
void update_stats();

#endif  // SYSTEM_MONITOR_H