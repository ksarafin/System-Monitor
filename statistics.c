#include <cairo.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "statistics.h"

#define MAX_NETWORK_CAPACITY (1.0 * 1e9)  // ASSUMPTION = 1 Gbps network interface

int g_num_samples = 0;
int g_delay_seconds = 0;
long int g_last_total_time = 0;
long int g_last_idle_time = 0;

GtkWidget *cpu_label;
GtkWidget *memory_label;
GtkWidget *swap_label;
GtkWidget *network_label;
GtkWidget *cpu_graph;
GtkWidget *memory_graph;
GtkWidget *swap_graph;
GtkWidget *network_graph;


/* Reads from /proc/meminfo, checks if certain fields are there, and calculates memory + swap usages */
void calculate_memory_swap_usage(double *memory_usage, double *swap_usage) {
  // Open /proc/meminfo for reading
  FILE *meminfo_file = fopen("/proc/meminfo", "r");
  if (meminfo_file == NULL) {
    perror("Error opening /proc/meminfo");
    exit(1);
  }

  // Variables to store memory and swap information
  long total_memory = 0;
  long free_memory = 0;
  long total_swap = 0;
  long free_swap = 0;
  char line[100];

  // Read lines from /proc/meminfo and extract relevant information
  while (fgets(line, sizeof(line), meminfo_file)) {
    if (sscanf(line, "MemTotal: %ld kB", &total_memory) == 1) {
      // Found total memory information
    } else if (sscanf(line, "MemFree: %ld kB", &free_memory) == 1) {
      // Found free memory information
    } else if (sscanf(line, "SwapTotal: %ld kB", &total_swap) == 1) {
      // Found total swap information
    } else if (sscanf(line, "SwapFree: %ld kB", &free_swap) == 1) {
      // Found free swap information
    }
  }
  // Close the file
  fclose(meminfo_file);

  // Calculate memory and swap usage percentages
  *memory_usage = ((total_memory - free_memory) * 100.0) / total_memory;
  *swap_usage = ((total_swap - free_swap) * 100.0) / total_swap;
}

gboolean draw_memory_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  // Retrieve memory usage value
  double memory_usage, swap_usage;
  calculate_memory_swap_usage(&memory_usage, &swap_usage);

  // Get the size of the drawing area
  int width, height;
  gtk_widget_get_size_request(widget, &width, &height);

  // Calculate the position for the line
  double x = width / 2.0;  // Center of the drawing area
  double y = height * (1 - memory_usage / 100.0);  // Adjust the scale

  // Set line color and width
  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // Red color
  cairo_set_line_width(cr, 2.0);

  // Move to the starting point
  cairo_move_to(cr, 0, height);

  // Draw X-axis
  cairo_line_to(cr, width, height);
  cairo_stroke(cr);

  // Move to the starting point
  cairo_move_to(cr, 0, 0);

  // Draw Y-axis
  cairo_line_to(cr, 0, height);
  cairo_stroke(cr);

  // Move to the starting point
  cairo_move_to(cr, 0, height);


  // Draw a line to the calculated position
  cairo_line_to(cr, x, y);

  // Stroke the path (draw the line)
  cairo_stroke(cr);

  return FALSE;
}

gboolean draw_swap_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  // Retrieve swap usage value
  double memory_usage, swap_usage;
  calculate_memory_swap_usage(&memory_usage, &swap_usage);

  // Get the size of the drawing area
  int width, height;
  gtk_widget_get_size_request(widget, &width, &height);

  // Calculate the position for the line
  double x = width / 2.0;  // Center of the drawing area
  double y = height * (1 - swap_usage / 100.0);  // Adjust the scale

  // Set line color and width
  cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);  // Green color
  cairo_set_line_width(cr, 2.0);

  // Move to the starting point
  cairo_move_to(cr, 0, height);

  // Draw x-axis
  cairo_line_to(cr, width, height);
  cairo_stroke(cr);

  // Move to the starting point
  cairo_move_to(cr, 0, 0);

  // Draw Y-axis
  cairo_line_to(cr, 0, height);
  cairo_stroke(cr);

  // Move to the starting point
  cairo_move_to(cr, 0, height);


  // Draw a line to the calculated position
  cairo_line_to(cr, x, y);

  // Stroke the path (draw the line)
  cairo_stroke(cr);

  return FALSE;
}

/* calculate network usage by reading from /proc/net/dev and taking the necessary info (parsing manually) */
double calculate_network_usage() {
  // Open /proc/net/dev for reading
  FILE *netdev_file = fopen("/proc/net/dev", "r");
  if (netdev_file == NULL) {
    perror("Error opening /proc/net/dev");
    exit(1);
  }

  // Variables to store network information
  char line[256];
  long received_bytes = 0;
  long transmitted_bytes = 0;

  // Skip the first two lines of headers in /proc/net/dev
  fgets(line, sizeof(line), netdev_file);
  fgets(line, sizeof(line), netdev_file);

  // Read lines from /proc/net/dev and extract relevant information
  if (fgets(line, sizeof(line), netdev_file) != NULL) {
    // Assuming there is at least one network interface
    sscanf(line, "%*s %ld %*d %*d %*d %*d %*d %*d %*d %*ld %*ld %*ld %*ld %*ld %*ld %*ld %*ld %ld",
        &received_bytes, &transmitted_bytes);
  } else {
    fprintf(stderr, "Error reading /proc/net/dev");
    exit(1);
  }
  // close the file
  fclose(netdev_file);

  // Print or store the network usage
  //printf("Received Bytes: %ld, Transmitted Bytes: %ld\n", received_bytes, transmitted_bytes);
  return ((received_bytes + transmitted_bytes) * 100.0) / MAX_NETWORK_CAPACITY;
}

gboolean draw_network_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  // Placeholder for network usage value (modify as per your network calculation)
  double network_usage = calculate_network_usage();

  // Get the size of the drawing area
  int width, height;
  gtk_widget_get_size_request(widget, &width, &height);

  // Calculate the position for the line
  double x = width / 2.0;  // Center of the drawing area
  double y = height * (1 - network_usage / 100.0);  // Adjust the scale

  // Set line color and width
  cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);  // Yellow color
  cairo_set_line_width(cr, 2.0);

  // Move to the starting point
  cairo_move_to(cr, 0, height);

  // Draw x-axis
  cairo_line_to(cr, width, height);
  cairo_stroke(cr);

  // Move to the starting point
  cairo_move_to(cr, 0, 0);

  // Draw Y-axis
  cairo_line_to(cr, 0, height);
  cairo_stroke(cr);

  // Move to the starting point
  cairo_move_to(cr, 0, height);


  // Draw a line to the calculated position
  cairo_line_to(cr, x, y);

  // Stroke the path (draw the line)
  cairo_stroke(cr);

  return FALSE;
}

/*
 * calculates cpu usage by tokenizing each line of the statistics file to
 * extract info for idle time -> helpful to know when to refresh the graphs
 */
double calculate_cpu_usage() {
  // variables to store information from /proc/stat
  char stat_line[100];
  const char delimiter[2] = " ";
  char *token = NULL;
  long int total_time = 0;
  long int idle_time = 0;
  double idle_percentage = 0.0;

  // open /proc/stat for reading
  FILE *stat_file = fopen("/proc/stat", "r");
  int field_index = 0;

  // read a line from /proc/stat
  fgets(stat_line, sizeof(stat_line), stat_file);
  fclose(stat_file);

  // tokenize the line using space as the delimiter
  token = strtok(stat_line, delimiter);
  total_time = 0;

  // process each token
  while (token != NULL) {
    token = strtok(NULL, delimiter);

    if (token != NULL) {
      total_time += atoi(token);
    }

    // check if the token corresponds to idle time
    if (field_index == 3) {
      idle_time = atoi(token);
    }

    field_index++;
  }

  // calculate idle percentage
  idle_percentage = 100 - (idle_time - g_last_idle_time) * 100.0 / (total_time - g_last_total_time);

  // update last values for the next iteration
  g_last_idle_time = idle_time;
  g_last_total_time = total_time;

  // return the calculated CPU usage percentage
  return 100 - idle_percentage;
}

gboolean draw_cpu_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
  // Retrieve CPU usage value
  double cpu_usage = calculate_cpu_usage();

  // Get the size of the drawing area
  int width, height;
  gtk_widget_get_size_request(widget, &width, &height);

  // Calculate the position for the line
  double x = width / 2.0;  // Center of the drawing area
  double y = height * (1 - cpu_usage / 100.0);  // Adjust the scale

  // Set line color and width
  cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);  // Blue color
  cairo_set_line_width(cr, 2.0);

  // Move to the starting point
  cairo_move_to(cr, 0, height);

  // Draw a line to the calculated position
  cairo_line_to(cr, x, y);

  // Stroke the path (draw the line)
  cairo_stroke(cr);

  return FALSE;
}

/* incorporating gtk where we take our calculations and update the GUI with them */
void update_stats() {
  double cpu_usage = 0.0;
  double memory_usage = 0.0;
  double swap_usage = 0.0;
  double network_usage = 0.0;

  // calculate CPU, memory, and swap usage
  cpu_usage = calculate_cpu_usage();
  calculate_memory_swap_usage(&memory_usage, &swap_usage);
  network_usage = calculate_network_usage();

  // updating labels and progress bars for:

  // 1. CPU Usage
  char text[100];
  snprintf(text, sizeof(text), "CPU Usage: %.2lf%%", cpu_usage);
  gtk_label_set_text(GTK_LABEL(cpu_label), text);
  draw_cpu_graph(cpu_graph, NULL, NULL);

  // 2. Memory Usage
  snprintf(text, sizeof(text), "Memory Usage: %.2lf%%", memory_usage);
  gtk_label_set_text(GTK_LABEL(memory_label), text);
  draw_memory_graph(memory_graph, NULL, NULL);

  // 3. Swap Usage
  snprintf(text, sizeof(text), "Swap Usage: %.2lf%%", swap_usage);
  gtk_label_set_text(GTK_LABEL(swap_label), text);
  draw_swap_graph(swap_graph, NULL, NULL);

  // 4. Network Usage
  snprintf(text, sizeof(text), "Network Usage: %.2lf%%", network_usage);
  gtk_label_set_text(GTK_LABEL(network_label), text);
  draw_network_graph(network_graph, NULL, NULL);
}

void create_graph(GtkWidget *grid, GtkWidget **graph, const gchar *label_text, GCallback draw_func) {
  // Label for the graph
  GtkWidget *graph_label = gtk_label_new(label_text);
  gtk_grid_attach(GTK_GRID(grid), graph_label, 0, 3, 1, 1);

  // Graph drawing area
  *graph = gtk_drawing_area_new();
  gtk_widget_set_size_request(*graph, 200, 100);  // Set the size as needed
  gtk_grid_attach(GTK_GRID(grid), *graph, 1, 3, 1, 1);

  // Connect the draw signal to the draw function
  g_signal_connect(*graph, "draw", draw_func, NULL);
}

/* main function -> format the gtk GUI, periodically updating it with refreshed stats */
int main(int argc, char *argv[]) {
  // check if the correct number of command-line arguments is provided
  if (argc != 3) {
    printf("Usage: %s <number of samples /proc/stat should be polled followed by delay in seconds>\n", argv[0]);
    return -1;
  }
  // parse command-line arguments
  g_num_samples = atoi(argv[1]);
  g_delay_seconds = atoi(argv[2]);

  // initialize GTK
  gtk_init(&argc, &argv);

  // main window
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "System Monitor");
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // grid for organizing widgets
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  gtk_container_add(GTK_CONTAINER(window), grid);

  // labels + progress bars for:

  // 1. CPU Usage
  cpu_label = gtk_label_new("CPU Usage:");
  gtk_grid_attach(GTK_GRID(grid), cpu_label, 0, 0, 1, 1);

  cpu_graph = gtk_drawing_area_new();
  gtk_widget_set_size_request(cpu_graph, 200, 100);
  gtk_grid_attach(GTK_GRID(grid), cpu_graph, 1, 0, 1, 1);
  g_signal_connect(cpu_graph, "draw", G_CALLBACK(draw_cpu_graph), NULL);

  // 2. Memory Usage
  memory_label = gtk_label_new("Memory Usage:");
  gtk_grid_attach(GTK_GRID(grid), memory_label, 0, 1, 1, 1);

  memory_graph = gtk_drawing_area_new();
  gtk_widget_set_size_request(memory_graph, 200, 100);
  gtk_grid_attach(GTK_GRID(grid), memory_graph, 1, 1, 1, 1);
  g_signal_connect(memory_graph, "draw", G_CALLBACK(draw_memory_graph), NULL);

  // 3. Swap Usage
  swap_label = gtk_label_new("Swap Usage:");
  gtk_grid_attach(GTK_GRID(grid), swap_label, 0, 2, 1, 1);

  swap_graph = gtk_drawing_area_new();
  gtk_widget_set_size_request(swap_graph, 200, 100);
  gtk_grid_attach(GTK_GRID(grid), swap_graph, 1, 2, 1, 1);
  g_signal_connect(swap_graph, "draw", G_CALLBACK(draw_swap_graph), NULL);

  // 4. Network Usage
  GtkWidget *network_label = gtk_label_new("Network Usage:");
  gtk_grid_attach(GTK_GRID(grid), network_label, 0, 4, 1, 1);

  GtkWidget *network_graph = gtk_drawing_area_new();
  gtk_widget_set_size_request(network_graph, 200, 100);
  gtk_grid_attach(GTK_GRID(grid), network_graph, 1, 3, 1, 1);
  g_signal_connect(network_graph, "draw", G_CALLBACK(draw_network_graph), NULL);

  // timer allowing us to update stats at certain time intervals
  guint timer_id = g_timeout_add_seconds(g_delay_seconds, (GSourceFunc)update_stats, window);

  // showing all the created widgets
  gtk_widget_show_all(window);

  gtk_main();

  // cleaning up our stuff
  g_source_remove(timer_id);
  return 0;
}
